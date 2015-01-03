#include <base/kmain.h>
#include <base/mem/alloc.h>
#include <base/module.h>
#include <base/kstd.h>
#include <base/elf.h>
#include <base/syscalls.h>
#include <base/vfs/vfs.h>

#include <base/initrd.h>
#include <base/tasking/task.h>
#include <base/tasking/userspace.h>

// XXX: remove after userspace loading is fully implemented
extern int syscall_spawn( int, char **, char **, int );

void utest( ){
	switch_to_usermode( );
	int fd;

	// open initial file descriptors
	fd = syscall_open( "/test/devices/keyboard", FILE_READ );
	fd = syscall_open( "/test/devices/console",  FILE_READ );
	fd = syscall_open( "/test/devices/console",  FILE_READ );

	fd = syscall_open( "/test/userroot/bin/sh", FILE_READ );
	if ( fd >= 0 ){
		kprintf( "[%s] Spawning process from fd %d\n", __func__, fd );
		syscall_spawn( fd, (char *[]){ "/test/userroot/asdf", "meh", 0 }, (char *[]){ "LOLENV=asdf", 0 }, 0 );
	}

	syscall_test( );

	while( 1 );
}

/** \brief The main kernel entry point, initializes things that are
 *         not architecture specific.
 *
 *  This function initializes modules, tasking, syscalls, and the userspace.
 *
 *  @param flags    (Currently undefined) flags passed from the \ref arch_init function
 *  @param modules  Location in memory of the init ramdisk, or NULL if none
 *  @param elfinfo  Location of the kernel's symbols, or NULL if none. If this is null,
 *                  the modules parameter must be ignored.
 */
//void kmain( multiboot_header_t *mboot, int blarg, int magic ){
void kmain( unsigned flags, void *modules, multiboot_elf_t *elfinfo ){
	initrd_t *initrd = 0;
	kprintf( "-==[ Helix kernel booting\n" );

	initrd = init_initrd( modules );

	init_syscalls( );
	init_tasking( );

	init_vfs( );

	// Initialize module system
	init_module_system( elfinfo );
	load_init_modules( initrd );

	//dump_aheap_blocks( kheap );

	kprintf( "-==[ Kernel initialized successfully.\n" );

	create_thread( utest );
}
