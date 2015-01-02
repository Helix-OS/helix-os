#ifndef _helix_logger_h
#define _helix_logger_h
#include <base/stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

char *logputs( char *str );
int kvprintf( char *format, va_list args );
int kprintf( char *format, ... );
int atoi( char *n );

#ifdef __cplusplus
}
#endif
#endif
