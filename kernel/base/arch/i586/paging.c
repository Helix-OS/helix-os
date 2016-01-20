// TODO: Make this source more readable
#ifndef _helix_paging_c
#define _helix_paginc_c
#include <arch/paging.h>
#include <base/logger.h>
#include <base/string.h>
#include <base/mem/alloc.h>
#include <base/bitmap.h>
#include <base/kstd.h>
#include <base/tasking/task.h>

int paging_enabled = 0;
unsigned int npages = 0, last_free_page = 0, nfree_pages = 0;
unsigned int *current_dir, *kernel_dir;
char *page_bitmap;

int init_paging( unsigned max_mem ){
	unsigned long i, j;
	register_interrupt_handler( 0xe, page_fault_handler );

	kernel_dir	= kmalloc_early( PAGE_SIZE, 1 );
	npages		= max_mem / 4;
	page_bitmap	= kmalloc_early( npages / 8, 1 );
	nfree_pages = npages;

	kprintf( "%d pages\n", npages );

	memset( page_bitmap, 0, npages / 8 );
	memset( kernel_dir, 0, PAGE_SIZE );

	for ( j = 0, i = PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT; i < 0x300000; i += 0x1000, j++ ){
		map_r_page( kernel_dir, i + KERNEL_VBASE, i, PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT );
		BM_SET_BIT( page_bitmap, ((i & ~0xfff ) >> 12), 1 );
		nfree_pages--;
	}

	current_dir = kernel_dir;
	paging_enabled = 1;
	set_page_dir( kernel_dir );

	return 0;
}

int map_page( unsigned *dir, unsigned vaddress, unsigned permissions ){
	int ret = 1;
	unsigned *move = (void *)0;
	unsigned raddress;

	raddress = get_free_page( );
	//raddress = (vaddress & 0xfff) | (raddress & ~0xfff);
	raddress = (raddress & ~0xfff) | permissions;

	if ( dir ){
		if ( !dir[ vaddress >> 22 ] ){
			void *newtable = ident_virt_to_phys( kmalloc_early( PAGE_SIZE, 1 ));
			dir[ vaddress >> 22] = (uintptr_t)newtable | permissions;
		}

		unsigned *ptable = ident_phys_to_virt( (void *)(dir[vaddress >> 22] & ~0xfff ));

		ptable[ (vaddress >> 12) & 0x3ff ] = raddress;

		kprintf( "mapped 0x%x, 0x%x, 0x%x (0x%x)\n", vaddress, (vaddress >> 12) & 0x3ff,
				 ptable[ (vaddress >> 12) & 0x3ff ], BM_GET_BIT( page_bitmap, (raddress >> 12)));
		BM_SET_BIT( page_bitmap, (raddress >> 12), 1 );
	}

	return ret;
}

int map_r_page( unsigned *dir, unsigned vaddress, unsigned raddress, unsigned permissions ){
	int ret = 0;
	unsigned *move = (void *)0;

	raddress = (raddress & ~0xfff) | permissions;

	if ( dir ){
		if ( !dir[ vaddress >> 22 ] ){
			void *newtable = ident_virt_to_phys( kmalloc_early( PAGE_SIZE, 1 ));
			dir[ vaddress >> 22] = (uintptr_t)newtable | permissions;
		}

		unsigned *ptable = ident_phys_to_virt( (void *)(dir[vaddress >> 22] & ~0xfff ));
		ptable[ (vaddress >> 12) & 0x3ff ] = raddress;

		ret = 1;
	}

	return ret;
}

int map_pages( unsigned *dir, unsigned start, unsigned end, unsigned permissions ){
	unsigned int i;

	//for ( i = start | permissions; i < end; i+= PAGE_SIZE )
	for ( i = start; i < end; i+= PAGE_SIZE )
		map_page( dir, i, permissions );

	return 1;
}

/** Change the permissions of already-mapped pages */
int remap_pages( unsigned *dir, unsigned start, unsigned end, unsigned permissions ){
	unsigned i;

	for ( i = start; i < end; i += PAGE_SIZE )
		map_r_page( dir, (i & ~(PAGE_SIZE - 1)), get_page( dir, i ), permissions);

	return 1;
}

int free_page( unsigned *dir, unsigned vaddress ){
	int ret = 0;
	unsigned int	*move = (void *)0,
			raddress = 0;

	if ( dir ){
		if ( dir[ vaddress >> 22 ]){
			unsigned *ptable = ident_phys_to_virt( (void *)( dir[vaddress >> 22] & ~0xfff ));
			raddress = ptable[ (vaddress >> 12) & 0x3ff ];
			ptable[ vaddress >> 12 & 0x3ff ] = 0;

			if ( raddress ){
				kprintf( "[%s] freeing 0x%x at 0x%x (0x%x)\n", __func__, vaddress, raddress, raddress >> 15 );
				BM_SET_BIT( page_bitmap, (raddress >> 12), 0 );

				if ( last_free_page > (raddress >> 15))
					last_free_page = (raddress >> 15);

				nfree_pages++;
				ret = 1;

			} else {
				kprintf( "[%s] /!\\ have null real address for 0x%x, not freeing...\n", __func__, vaddress );
				ret = 0;
			}
		}
	}

	return ret;
}

unsigned get_page( unsigned *dir, unsigned vaddress ){
	unsigned ret = 0;
	unsigned *move = (void *)0;

	if ( dir ){
		unsigned *ptable = ident_phys_to_virt( (void *)(dir[vaddress >> 22] & ~0xfff ));

		if ( ptable ){
			move = (unsigned *)((uintptr_t)ptable & ~0xfff);
			ret = move[ vaddress >> 12 & 0x3ff ];
		}
	}
	
	return ret;
}

unsigned get_free_page( ){
	unsigned ret = 0;
	unsigned i;
	unsigned j;
	unsigned page;

	if ( page_bitmap ){
		for ( page = last_free_page;( page_bitmap[ page ] & 0xff ) == 0xff; page++ )
			kprintf( "." );

		last_free_page = page;
		i = page_bitmap[ page ] & 0xff;
		
		kprintf( "0x%x: ", i );
		for ( j = 0; i & 1; i>>=1 ) j++;
		ret = ((page << 3) + j) << 12;
		nfree_pages--;

		kprintf( "get_free_page: returning 0x%x, 0x%x, 0x%x, %d pages remaining\n", ret, page, j, nfree_pages );
	}
	
	return ret;
}

// get number of free pages
unsigned get_nfree_pages( ){
	return nfree_pages;
}

page_dir_t *get_current_page_dir( ){
	return current_dir;
}

page_dir_t *get_kernel_page_dir( ){
	return kernel_dir;
}

void set_page_dir( unsigned *dir ){
	unsigned	address = 0,
			cr0;

	if ( paging_enabled )
		address = get_page( current_dir, (unsigned)dir );
	else
		address = (unsigned)dir;

	if ( !address ){
		kprintf( "Got bad address [panic here]\n" );
	}

	address = address | PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT;

	current_dir = dir;
	asm volatile( "mov %0, %%cr3":: "r"( address ));
	asm volatile( "mov %%cr0, %0": "=r"( cr0 ));
	asm volatile( "mov %0, %%cr0":: "r"( cr0 | 0x80000000 ));
}

unsigned *clone_page_dir( unsigned *dir ){
	unsigned *ret;
	unsigned i;

	ret = kmalloca( PAGE_SIZE );
	kprintf( "[%s] allocated new page dir at 0x%x, physical location 0x%x\n", __func__, ret,
			get_page( current_dir, (unsigned)ret ));

	for( i = 0; i < PAGE_SIZE / 4; i++ )
		ret[i] = dir[i];

	return ret;
}

void flush_tlb( ){
	asm volatile( "mov %cr3, %eax" );
	asm volatile( "mov %eax, %cr3" );
}

void page_fault_handler( registers_t *regs ){
	unsigned long fault_addr;
	task_t *current;
	list_node_t *temp;
	memmap_t *map;
	bool found = false;

	asm volatile( "mov %%cr2, %0": "=r"( fault_addr ));
	kprintf( "page fault on 0x%x\n", fault_addr );
	dump_registers( regs );

	current = get_current_task( );
	if ( !current || !current->pid ){
		panic( "page fault: 0x%x: 0x%x:%s%s%s\n",
			fault_addr, regs->err_code,
			(!regs->err_code & 1 )?		" not present":"",
			( regs->err_code & 2 )?		" readonly":"",
			( regs->err_code & 4 )?		" usermode":""
		);

	} else {
		if ( current->memmaps ){
			temp = current->memmaps->base;
			foreach_in_list( temp ){
				map = temp->data;
				if ( memmap_check( map, fault_addr )){
					found = true;
					break;
				}
			}
		}

		if ( found ){
			map_page( current->pagedir,
					(fault_addr & ~(PAGE_SIZE  - 1)), PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT );

			kprintf( "================ [%s] Mapped page 0x%x\n", __func__, 
					(fault_addr & ~(PAGE_SIZE  - 1)) | PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT );
			flush_tlb( );

		} else {
			kprintf( "page fault: 0x%x: 0x%x:%s%s%s\n",
				fault_addr, regs->err_code,
				(!regs->err_code & 1 )?		" not present":"",
				( regs->err_code & 2 )?		" readonly":"",
				( regs->err_code & 4 )?		" usermode":""
			);

			kprintf( "[%s] Have bad process %d, killing...\n", __func__, current->pid );
			if ( current->sem ){
				*current->sem++;
				kprintf( "[%s] Resetting semaphore...\n", __func__, current->pid );
			}
			remove_task_by_pid( current->pid );
		}
	}
}

#endif
