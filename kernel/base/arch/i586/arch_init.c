#include <arch/init_tables.h>
#include <arch/paging.h>
#include <arch/pitimer.h>
#include <base/multiboot.h>
#include <base/kmain.h>
#include <base/mem/alloc.h>

extern unsigned int *kernel_dir;
extern unsigned int early_placement;
extern mheap_t *kheap;
multiboot_header_t *bootheader = 0;

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
	void *elfinfo = 0;

	init_serial( );

	// Take care of multiboot stuff...
	if ( magic != 0x2badb002 )
		panic( "Need multiboot-compliant bootloader to boot Helix kernel.\n" );

	bootheader = mboot;

	if ( mboot->flags & MULTIBOOT_FLAG_ELF )
		elfinfo = &mboot->elf_headers;

	if ( mboot->flags & MULTIBOOT_FLAG_MODS && mboot->mods_count ){
		modules = *(int **)mboot->mods_addr;
		early_placement = *(int *)(mboot->mods_addr + 4);
	}

	// Set up memory utils
	init_tables( );
	init_paging( mboot->mem_lower + mboot->mem_upper );

	kheap = kmalloc_early( sizeof( mheap_t ), 0 );
	init_heap( kheap, kernel_dir, 0xd0000000, PAGE_SIZE * 32 );

	init_timer( );

	kmain( 0, modules, elfinfo );

	while( 1 ) asm volatile( "hlt" );
}
