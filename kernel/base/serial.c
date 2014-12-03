#include <base/stdio.h>
#include <base/string.h>
#include <base/serial.h>
#include <base/tasking/semaphore.h>

#define PORT 0x3f8
static semaphore_t serial_sem = 1;

void init_serial( ){
	outb( PORT + 1, 0x00 );
	outb( PORT + 3, 0x80 );
	outb( PORT + 0, 0x03 );
	outb( PORT + 1, 0x00 );
	outb( PORT + 3, 0x03 );
	outb( PORT + 2, 0xc7 );
	outb( PORT + 4, 0x0b );
}

bool serial_recieved( ){
	return inb( PORT + 5 ) & 1;
}

int transmit_empty( ){
	return inb( PORT + 5 ) & 0x20;
}

unsigned int read_serial( void *buf, unsigned size ){
	char *in = buf;
	unsigned i;

	enter_semaphore( &serial_sem );

	for ( i = 0; i < size; i++ ){
		while( !serial_recieved( ))
			asm volatile( "hlt" );

		in[i] = inb( PORT );
	}

	leave_semaphore( &serial_sem );
	return i;
}

unsigned int write_serial( void *buf, unsigned size ){
	char *in = buf;
	unsigned i;

	enter_semaphore( &serial_sem );

	for ( i = 0; i < size; i++ ){
		while( !transmit_empty( ))
			asm volatile( "hlt" );

		outb( PORT, in[i] );
	}

	leave_semaphore( &serial_sem );
	return i;	
}
