#ifndef _kernel_panic_c
#define _kernel_panic_c
#include <base/kstd.h>

void panic( char *fmt, ... ){
	va_list args;
	va_start( args, fmt );

	kprintf( "\n\n- = - = - = -[ PANIC ]- = - = - = -\n" );
	kvprintf( fmt, args );
	kprintf( "\n- = - = - = - = - = - = - = - = - =\n" );
	asm volatile( "cli" );
	asm volatile( "hlt" );
}

#endif
