#include <base/kmain.h>
#include <base/arch/i386/init_tables.h>
#include <base/arch/i386/paging.h>
#include <base/multiboot.h>
#include <base/mem/alloc.h>
#include <base/module.h>
#include <base/kstd.h>
#include <base/elf.h>
#include <base/syscalls.h>
#include <vfs/vfs.h> // TODO: Move this to the base tree

#include <base/initrd.h>

#include <base/arch/i386/pitimer.h>
#include <base/tasking/task.h>
#include <base/tasking/userspace.h>

extern unsigned int *kernel_dir;
extern mheap_t *kheap;
extern unsigned int early_placement;

// XXX: remove after userspace loading is fully implemented
extern int syscall_spawn( int, char **, char **, int );

void utest( ){
	switch_to_usermode( );
	int fd;

	// open initial file descriptors
	fd = syscall_open( "/test/devices/keyboard", FILE_READ );
	fd = syscall_open( "/test/devices/console",  FILE_READ );
	fd = syscall_open( "/test/devices/console",  FILE_READ );

	fd = syscall_open( "/test/userroot/bin/init", FILE_READ );
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
 *  @param mboot The multiboot header passed by a compliant bootloader.
 *  @param blarg At the moment, the initial stack placement. This parameter
 *               is unused and will be removed soon.
 *  @param magic The multiboot header checksum.
 */
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

	init_vfs( );

	// Initialize module system
	init_module_system( elfinfo );
	load_init_modules( initrd );

	dump_aheap_blocks( kheap );

	kprintf( "-==[ Kernel initialized successfully.\n" );
	asm volatile( "int $0x30" );

	create_thread( utest );

	while( 1 ) asm volatile( "hlt" );
}
