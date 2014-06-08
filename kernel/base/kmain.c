#include <base/kmain.h>
#include <base/arch/i386/init_tables.h>
#include <base/arch/i386/paging.h>
#include <base/multiboot.h>
#include <base/mem/alloc.h>
#include <base/module.h>
#include <base/kstd.h>
#include <base/elf.h>
#include <base/hal.h>
#include <base/syscalls.h>
#include <vfs/vfs.h> // TODO: Move this to the base tree

#include <base/initrd.h>

#include <base/arch/i386/pitimer.h>
#include <base/tasking/task.h>
#include <base/tasking/userspace.h>

extern unsigned int *kernel_dir;
extern mheap_t *kheap;
extern unsigned int early_placement;

void utest( ){
	switch_to_usermode( );

	int fd;
	fd = syscall_open( "/test/fatdir/asdf", FILE_READ );
	if ( fd >= 0 )
		syscall_spawn( fd, (char *[]){ "/test/fatdir/asdf", "meh", 0 }, (char *[]){ "LOLENV=asdf", 0 }, 0 );

	//syscall_test( );

	while( 1 );
}

void kmain( multiboot_header_t *mboot, int blarg, int magic ){
	void *modules;
	multiboot_elf_t *elfinfo = 0;
	initrd_t *initrd;

	kprintf( "-==[ Helix kernel booting\n" );

	// Take care of multiboot stuff...
	if ( magic != 0x2badb002 )
		panic( "Need multiboot-compliant bootloader to boot Helix kernel.\n" );

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

	initrd = init_initrd( modules );

	asm volatile( "sti" );
	init_syscalls( );
	init_pitimer( 1000 );
	init_tasking( );

	init_hal( );
	init_vfs( );

	// Initialize module system
	init_module_system( elfinfo );
	load_init_modules( initrd );

	hal_dump_devices( );
	dump_aheap_blocks( kheap );

	kprintf( "-==[ Kernel initialized successfully.\n" );
	asm volatile( "int $0x30" );

	create_thread( utest );

	while( 1 ) asm volatile( "hlt" );
}
