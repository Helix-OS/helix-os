#ifndef _helix_stdio_h
#define _helix_stdio_h

unsigned char inb( unsigned short port );
void outb( unsigned short port, unsigned char data );

unsigned long inl( unsigned short port );
void outl( unsigned short port, unsigned long data );

#endif
