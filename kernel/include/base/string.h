#ifndef _helix_string_h
#define _helix_string_h
#include <base/mem/alloc.h>

unsigned strlen( char * );
int strcmp( char *, char * );
int strncmp( char *, char *, int );
char *strdup( char * );

void *memset( void *, unsigned char, unsigned );
void *memsetw( void *, unsigned char, unsigned );
void *memcpy( void *, void *, unsigned );
void *memmove( void *, void *, unsigned );

#endif
