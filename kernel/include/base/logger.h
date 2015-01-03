#ifndef _helix_logger_h
#define _helix_logger_h
#include <base/stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *logputs( const char *str );
int kvprintf( const char *format, va_list args );
int kprintf( const char *format, ... );
int atoi( const char *n );

#ifdef __cplusplus
}
#endif
#endif
