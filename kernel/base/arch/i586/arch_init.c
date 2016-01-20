#include <arch/init_tables.h>
#include <arch/paging.h>
#include <arch/pitimer.h>
#include <base/multiboot.h>
#include <base/kmain.h>
#include <base/mem/alloc.h>
#include <base/mem/salloc.h>

#include <base/elf.h>

extern unsigned int *kernel_dir;
extern unsigned int early_placement;
//extern mheap_t *kheap;
multiboot_header_t *bootheader = 0;

/* XXX: This manually changes section headers given by grub so they point to
 *      identity-mapped virtual addresses
 *
 *      Maybe this would be better somewhere else, not sure where to put it though,
 *      since it deals with multiboot-specific things
 */
static void adjust_mboot_elf_headers( multiboot_elf_t *elfinfo ){
	unsigned i;
	Elf32_Shdr *shdr;

	for ( i = 0 ; i < elfinfo->num; i++ ){
		shdr = (Elf32_Shdr *)((unsigned long)elfinfo->addr + elfinfo->size * i );

		if ( shdr->sh_addr && shdr->sh_addr < KERNEL_VBASE ){
			Elf32_Addr temp = shdr->sh_addr + KERNEL_VBASE;
			kprintf( "[%s] Adjusting 0x%x -> 0x%x\n", __func__, shdr->sh_addr, temp );
			shdr->sh_addr = temp;

		} else {
			kprintf( "[%s] Leaving %x...\n", __func__, shdr->sh_addr );
		}
	}
}

static void adjust_mboot_vbe_info( multiboot_header_t *header ){
	uintptr_t oldmode = header->vbe_mode_info;
	uintptr_t oldcontrol = header->vbe_control_info;

	header->vbe_mode_info =
		(uintptr_t)ident_phys_to_virt((void *)header->vbe_mode_info );
	kprintf( "[%s] Adjusting 0x%x to 0x%x\n", __func__, oldmode, header->vbe_mode_info );

	header->vbe_control_info =
		(uintptr_t)ident_phys_to_virt((void *)header->vbe_control_info );
	kprintf( "[%s] Adjusting 0x%x to 0x%x\n", __func__, oldcontrol, header->vbe_control_info );
}

/* \brief Handles archetecture-specific initialization for i586. 
 *
 *  This function initializes various tables, paging, and the initial heap,
 *  various timers, then passes the locations of modules and elf info (if any)
 *  to \ref kmain.
 *
 *  @param mboot The multiboot header passed by a compliant bootloader.
 *  @param blarg At the moment, the initial stack placement. This parameter
 *               is unused and will be removed soon.
 *  @param magic The multiboot header checksum.
 */
void arch_init( multiboot_header_t *mboot, int blarg, int magic ){
	void *modules = 0;
	multiboot_elf_t *elfinfo = 0;

	init_serial( );

	// Take care of multiboot stuff...
	if ( magic != 0x2badb002 )
		panic( "Need multiboot-compliant bootloader to boot Helix kernel.\n" );

	bootheader = mboot;

	if ( mboot->flags & MULTIBOOT_FLAG_ELF )
		elfinfo = &mboot->elf_headers;

	if ( mboot->flags & MULTIBOOT_FLAG_MODS && mboot->mods_count ){
		modules = ident_phys_to_virt( (*(int **)mboot->mods_addr ));
		early_placement = (uintptr_t)ident_phys_to_virt((void *)(*(int *)(mboot->mods_addr + 4)));
	}

	// Set up memory utils
	init_tables( );
	init_paging( mboot->mem_lower + mboot->mem_upper );
	init_kernel_heap( kernel_dir, 0xd0000000, PAGE_SIZE * 128 );
	init_timer( );

	kprintf( "[%s] Adjusting multiboot elf info...\n", __func__ );
	elfinfo->addr += KERNEL_VBASE;
	adjust_mboot_elf_headers( elfinfo );
	adjust_mboot_vbe_info( mboot );

	kmain( 0, modules, elfinfo, ident_phys_to_virt( (void *)mboot->cmdline ));

	while( 1 ) asm volatile( "hlt" );
}
