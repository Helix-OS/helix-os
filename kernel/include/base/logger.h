#ifndef _helix_logger_h
#define _helix_logger_h
#include <base/stdarg.h>

void init_logger( );
int transmit_empty( );
unsigned write_logger( void *buf, unsigned size );
char *logputs( char *str );
int kvprintf( char *format, va_list args );
int kprintf( char *format, ... );

#endif
