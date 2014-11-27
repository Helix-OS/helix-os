#ifndef _helix_logger_h
#define _helix_logger_h
#include <base/stdarg.h>

char *logputs( char *str );
int kvprintf( char *format, va_list args );
int kprintf( char *format, ... );
int atoi( char *n );

#endif
