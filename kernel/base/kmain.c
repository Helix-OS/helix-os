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

#include <base/arch/i386/pitimer.h>
#include <base/tasking/task.h>
#include <base/tasking/userspace.h>

extern unsigned int *kernel_dir;
extern mheap_t *kheap;
extern unsigned int early_placement;

void sometest( ){
	int counter = 0;
	extern unsigned *current_dir;
	set_page_dir( clone_page_dir( current_dir ));
	while( counter < 10 ){
		kprintf( "%d... ", counter++ );
		usleep( 1000 );
	}

	exit_thread( );
}

void userspace_test( ){
	int fd;
	char meh[512];
	syscall_test( );

	//fd = syscall_open( "/test/devices/device0", FILE_READ );
	fd = syscall_open( "/test/fatdir/README.md", FILE_READ );
	if ( fd >= 0 ){
		syscall_read( fd, meh, 512 );
		syscall_close( fd );

		fd = syscall_open( "/test/devices/device2", FILE_WRITE );
		syscall_write( fd, meh, 512 );
	}

	while( 1 );
}

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
	init_syscalls( );
	init_pitimer( 1000 );
	init_tasking( );

	//create_thread( sometest );

	init_hal( );
	init_vfs( );

	// Initialize module system
	init_module_system( elfinfo );
	load_init_modules((void *)modules );

	{
		/*
		int fd;
		char *mehstr = "Testing this thing";
		syscall_test( );

		fd = syscall_open( "/test/devices/device2", FILE_WRITE );
		syscall_write( fd, mehstr, strlen( mehstr ));

		char meh[512];

		fd = syscall_open( "/test/meh.txt", FILE_READ );
		if ( fd >= 0 ){
			syscall_read( fd, meh, 512 );
			syscall_close( fd );

			kprintf( "[%s] get %s\n", __func__, meh );

			fd = syscall_open( "/test/devices/device2", FILE_WRITE );
			syscall_write( fd, meh, 512 );
		}
		*/
	}

	hal_dump_devices( );
	dump_aheap_blocks( kheap );

	kprintf( "-==[ Kernel initialized successfully.\n" );
	asm volatile( "int $0x30" );
	create_thread( userspace_test );

	while( 1 ) asm volatile( "hlt" );
}
