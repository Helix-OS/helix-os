#ifndef _helix_std_h
#define _helix_std_h
#include <base/logger.h>

#define knew( N ) (kcalloc( 1, sizeof( N )))
#define NULL (void *)0
void panic( char *fmt, ... );

#endif
