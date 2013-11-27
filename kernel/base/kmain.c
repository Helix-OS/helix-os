#include <base/kmain.h>
#include <base/logger.h>
#include <base/arch/i386/init_tables.h>
#include <base/arch/i386/paging.h>
#include <base/multiboot.h>
#include <base/mem/alloc.h>

extern unsigned int *kernel_dir;
extern mheap_t *kheap;

void kmain( multiboot_header_t *mboot ){
	int	*buf,
		*temp,
		*stuff, i;

	kprintf( "-==[ Helix kernel booting\n" );
	init_tables( );
	init_paging( mboot->mem_lower + mboot->mem_upper );

	kheap = kmalloc_early( sizeof( mheap_t ), 0 );
	init_heap( kheap, kernel_dir, 0xc0000000, PAGE_SIZE * 8 );

	/*
	buf = kmalloc( sizeof( int ), 0 );
	*buf = 1234;
	kprintf( "buf: 0x%x\n", buf );

	temp = kmalloc( sizeof( int ), 0 );
	kprintf( "temp: 0x%x\n", temp );

	stuff = kmalloc( sizeof( int ), 0 );
	kprintf( "stuff: 0x%x\n", stuff );

	kfree( stuff );
	kfree( temp );

	temp = kmalloc( sizeof( int ), 0 );
	kprintf( "temp: 0x%x\n", temp );
	kprintf( "buf: %d\n", *buf );

	kfree( temp );
	kfree( buf );
	buf = kmalloc( sizeof( int ), 0 );
	*/

	kprintf( "--------- kmalloc tests ----------\n" );
	buf = kmalloc( sizeof( int ), 0 );
	kprintf( "buf: 0x%x\n", buf );

	buf = kmalloc( sizeof( int ), 0 );
	kprintf( "buf: 0x%x\n", buf );

	temp = kmalloc( sizeof( int ), 1 );
	kprintf( "temp: 0x%x\n", temp );

	temp = kmalloc( sizeof( int ), 1 );
	kprintf( "temp: 0x%x\n", temp );

	buf = kmalloc( sizeof( int ), 0 );
	kprintf( "buf: 0x%x\n", buf );

	temp = kmalloc( sizeof( int ), 1 );
	kprintf( "temp: 0x%x\n", temp );

	temp = kmalloc( sizeof( int ), 1 );
	kprintf( "temp: 0x%x\n", temp );

	buf = kmalloc( 300, 0 );
	kprintf( "buf: 0x%x\n", buf );

	kprintf( "-==[ Kernel initialized successfully.\n" );
	while( 1 ) asm volatile( "hlt" );
}
