#include <base/kmain.h>
#include <base/cmdline.h>
#include <base/mem/alloc.h>
#include <base/module.h>
#include <base/kstd.h>
#include <base/elf.h>
#include <base/syscalls.h>
#include <base/vfs/vfs.h>

#include <base/initrd.h>
#include <base/tasking/task.h>
#include <base/tasking/userspace.h>

initrd_t *main_initrd = 0;
cmdline_opt_t *main_opts = 0;

// XXX: remove after userspace loading is fully implemented
extern int syscall_spawn( int, char **, char **, int );

void utest( ){
	switch_to_usermode( );
	int fd;

	// open initial file descriptors
	syscall_open( "/test/devices/keyboard", FILE_READ );
    /*
	fd = syscall_open( "/test/devices/console",  FILE_READ );
	fd = syscall_open( "/test/devices/console",  FILE_READ );
    */

	// XXX: this references data in kernel address space, won't work once proper
	//      memory seperation is enforced. 
	// TODO:  add sysinfo request to get kernel options
	bool foo = cmdline_has( main_opts, "textmode" );
	if ( foo ){
		syscall_open( "/test/devices/console",  FILE_READ );
		syscall_open( "/test/devices/console",  FILE_READ );

	} else {
		syscall_open( "/test/devices/fbconsole",  FILE_READ );
		syscall_open( "/test/devices/fbconsole",  FILE_READ );
	}

	fd = syscall_open( "/test/userroot/bin/init", FILE_READ );
	//fd = syscall_open( "/test/userroot/bin/sh", FILE_READ );
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
 *  Architecture-specific initialization is done in \ref arch_init for the platform.
 *
 *  @param flags    (Currently undefined) flags passed from the \ref arch_init function
 *  @param modules  Location in memory of the init ramdisk, or NULL if none
 *  @param elfinfo  Location of the kernel's symbols, or NULL if none. If this is null,
 *                  the modules parameter must be ignored.
 */
// TODO: possibly change type of elfinfo to not depend on multiboot
void kmain( unsigned flags, void *modules, multiboot_elf_t *elfinfo, char *cmdline ){
	initrd_t *initrd = 0;
	kprintf( "-==[ Helix kernel booting\n" );
	cmdline_opt_t *opts = NULL;

	main_opts = parse_command_line( cmdline );
	initrd = init_initrd( modules );
    main_initrd = initrd;

	init_syscalls( );
	init_tasking( );

	init_vfs( );

	// Initialize module system
	init_module_system( elfinfo );
	load_init_modules( initrd );

	kprintf( "-==[ Kernel initialized successfully.\n" );

	create_thread( utest );
}
