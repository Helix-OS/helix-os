#ifndef _helix_serial_h
#define _helix_serial_h
#include <base/lib/stdbool.h>

void init_serial( );
int transmit_empty( );
bool serial_recieved( );
unsigned write_serial( void *buf, unsigned size );
unsigned int read_serial( void *buf, unsigned size );

#endif
