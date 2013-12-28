#ifndef _helix_stdio_c
#define _helix_stdio_c
#include <base/stdio.h>

unsigned char inb( unsigned short port ){
	unsigned char ret;
	asm volatile( "inb %1, %0" : "=a"( ret ) : "Nd"( port ));
	return ret;
}

void outb( unsigned short port, unsigned char data ){
	asm volatile( "outb %1, %0" :: "dN"( port ), "a"( data ));
}

unsigned long inl( unsigned short port ){
	unsigned long ret;
	asm volatile( "inl %1, %0" : "=a"( ret ) : "Nd"( port ));
	return ret;
}

void outl( unsigned short port, unsigned long data ){
	asm volatile( "outl %1, %0" :: "dN"( port ), "a"( data ));
}

#endif
