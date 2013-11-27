#ifndef _helix_paging_c
#define _helix_paginc_c
#include <base/arch/i386/paging.h>
#include <base/logger.h>
#include <base/string.h>
#include <base/mem/alloc.h>
#include <base/bitmap.h>
#include <base/kstd.h>

int paging_enabled = 0;
unsigned int 	npages = 0,
		last_free_page = 0;
unsigned int	*current_dir,
		*kernel_dir;
char *page_bitmap;

int init_paging( unsigned int max_mem ){
	unsigned long i, j;
	register_interrupt_handler( 0xe, page_fault_handler );

	kernel_dir	= kmalloc_early( PAGE_SIZE, 1 );
	npages		= max_mem / 4;
	page_bitmap	= kmalloc_early( npages / 8, 1 );

	kprintf( "%d pages\n", npages );

	memset( page_bitmap, 0, npages / 8 );
	memset( kernel_dir, 0, PAGE_SIZE );

	for ( j = 0, i = PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT; i < 0x300000; i += 0x1000, j++ ){
		map_r_page( kernel_dir, i, i );
		BM_SET_BIT( page_bitmap, ((i & ~0xfff ) >> 12), 1 );
	}

	set_page_dir( kernel_dir );
	paging_enabled = 1;

	return 0;
}

int map_page( unsigned int *dir, unsigned int vaddress ){
	int ret = 1;
	unsigned int	*move = (void *)0, 
			raddress;

	raddress = get_free_page( );
	raddress = (vaddress & 0xfff) | (raddress & ~0xfff);

	if ( dir ){
		if ( !dir[ vaddress >> 22 ] ){
			dir[ vaddress >> 22 ] = (unsigned)kmalloc_early( PAGE_SIZE, 1 ) | (vaddress & 0xfff);
		}

		move = (unsigned *)( dir[ vaddress >> 22 ] & ~0xfff );
		move[ (vaddress >> 12) & 0x3ff ] = raddress;
		kprintf( "mapped 0x%x, 0x%x, 0x%x (0x%x)\n", vaddress, (vaddress >> 12) & 0x3ff,
							 move[ (vaddress >> 12) & 0x3ff ], BM_GET_BIT( page_bitmap, (raddress >> 12)));
		BM_SET_BIT( page_bitmap, (raddress >> 12), 1 );
	}

	return ret;
}

int map_r_page( unsigned int *dir, unsigned int vaddress, unsigned int raddress ){
	int ret = 0;
	unsigned int *move = (void *)0;

	raddress = (vaddress & 0xfff) | (raddress & ~0xfff);

	if ( dir ){
		if ( !dir[ vaddress >> 22 ] ){
			dir[ vaddress >> 22 ] = (unsigned)kmalloc_early( PAGE_SIZE, 1 ) | (vaddress & 0xfff);
		}

		move = (unsigned *)( dir[ vaddress >> 22 ] & ~0xfff );
		move[ vaddress >> 12 & 0x3ff ] = raddress;
		ret = 1;
	}

	return ret;
}

int map_pages( unsigned int *dir, unsigned int start, unsigned int end, unsigned int permissions ){
	unsigned int i;

	for ( i = start | permissions; i < end; i+= PAGE_SIZE )
		map_page( dir, i );

	return 1;
}

int free_page( unsigned int *dir, unsigned int vaddress ){
	int ret = 0;
	unsigned int	*move = (void *)0,
			raddress = 0;

	if ( dir ){
		if ( dir[ vaddress >> 22 ]){
			move = (unsigned *)( dir[ vaddress >> 22 ] & ~0xfff );
			raddress = move[ vaddress >> 12 & 0x3ff ];
			move[ vaddress >> 12 & 0x3ff ] = 0;
			kprintf( "freeing 0x%x at 0x%x (0x%x)\n", vaddress, raddress, raddress >> 15 );
			BM_SET_BIT( page_bitmap, (raddress >> 12), 0 );

			if ( last_free_page > (raddress >> 15))
				last_free_page = (raddress >> 15);

			ret = 1;
		}
	}

	return ret;
}

unsigned int get_page( unsigned int *dir, unsigned int vaddress ){
	unsigned int ret = 0;
	unsigned int *move = (void *)0;

	if ( dir ){
		move = (unsigned *)dir[ vaddress >> 22 ];
		if ( move ){
			move = (unsigned int *)((unsigned int)move & ~0xfff);
			ret = move[ vaddress >> 12 & 0x3ff ];
		}
	}
	
	return ret;
}

unsigned int get_free_page( ){
	unsigned int	ret = 0,
			i, j,
			page;

	if ( page_bitmap ){
		for ( page = last_free_page;( page_bitmap[ page ] & 0xff ) == 0xff; page++ )
			kprintf( "." );

		last_free_page = page;
		i = page_bitmap[ page ] & 0xff;
		
		kprintf( "0x%x: ", i );
		for ( j = 0; i & 1; i>>=1 ) j++;
		ret = ((page << 3) + j) << 12;
		kprintf( "get_free_page: returning 0x%x, 0x%x, 0x%x\n", ret, page, j );
	}
	
	return ret;
}

void set_page_dir( unsigned int *dir ){
	unsigned int	address = 0,
			cr0;

	if ( paging_enabled )
		address = get_page( current_dir, (unsigned int)dir );
	else
		address = (unsigned int)dir;

	if ( !address ){
		kprintf( "Got bad address [panic here]\n" );
	}

	kprintf( "Setting page dir to 0x%x... ", address );

	current_dir = dir;
	asm volatile( "mov %0, %%cr3":: "r"( address ));
	asm volatile( "mov %%cr0, %0": "=r"( cr0 ));
	asm volatile( "mov %0, %%cr0":: "r"( cr0 | 0x80000000 ));
}

void flush_tlb( ){
	asm volatile( "mov %cr3, %eax" );
	asm volatile( "mov %eax, %cr3" );
}

void page_fault_handler( registers_t *regs ){
	unsigned long fault_addr;

	asm volatile( "mov %%cr2, %0": "=r"( fault_addr ));
	panic( "page fault: 0x%x:%s%s%s\n",
		fault_addr,
		(!regs->err_code & 1 )?		" not present":"",
		( regs->err_code & 2 )?		" readonly":"",
		( regs->err_code & 4 )?		" usermode":""
	);
}

#endif
