#include <base/kmain.h>
#include <base/arch/i386/init_tables.h>
#include <base/arch/i386/paging.h>
#include <base/multiboot.h>
#include <base/mem/alloc.h>
#include <base/module.h>
#include <base/kstd.h>
#include <base/elf.h>
#include <base/hal.h>

#include <base/arch/i386/pitimer.h>

extern unsigned int *kernel_dir;
extern mheap_t *kheap;
extern unsigned int early_placement;

void kmain( multiboot_header_t *mboot, int blarg, int magic ){
	int *modules;
	multiboot_elf_t *elfinfo = 0;

	kprintf( "-==[ Helix kernel booting\n" );

	// Take care of multiboot stuff...
	if ( magic != 0x2badb002 )
		panic( "Need multiboot-compliant bootloader to boot Helix kernel.\n" );

	if ( mboot->flags & MULTIBOOT_FLAG_ELF )
		elfinfo = &mboot->elf_headers;

	if ( mboot->flags & MULTIBOOT_FLAG_MODS && mboot->mods_count ){
		modules = (int *)*(int *)mboot->mods_addr;
		early_placement = *(int *)(mboot->mods_addr + 4);
	}

	// Set up memory utils
	init_tables( );
	init_paging( mboot->mem_lower + mboot->mem_upper );

	kheap = kmalloc_early( sizeof( mheap_t ), 0 );
	init_heap( kheap, kernel_dir, 0xd0000000, PAGE_SIZE * 8 );

	asm volatile( "sti" );
	init_pitimer( 100 );

	// Initialize module system
	init_module_system( elfinfo );
	load_init_modules((void *)modules );

	hal_dump_devices( );
	dump_aheap_blocks( kheap );

	kprintf( "-==[ Kernel initialized successfully.\n" );
	while( 1 ) asm volatile( "hlt" );
}
