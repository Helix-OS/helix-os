#ifndef _helix_string_h
#define _helix_string_h
#include <base/mem/alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned strlen( const char * );
int strcmp( const char *, const char * );
int strncmp( const char *, const char *, int );
char *strdup( const char * );

char *strcpy( char *dest, const char *src );
char *strncpy( char *dest, const char *src, unsigned len );

char *strcat( char *dest, const char *src );
char *strncat( char *dest, const char *src, int n );

void *memset( void *, unsigned char, unsigned );
void *memsetw( void *, unsigned char, unsigned );
void *memcpy( void *, const void *, unsigned );
void *memmove( void *, const void *, unsigned );
int memcmp( const void *s1, const void *s2, unsigned len );

#ifdef __cplusplus
}
#endif
#endif
