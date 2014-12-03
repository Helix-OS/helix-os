#ifndef _kernel_string_c
#define _kernel_string_c
#include <base/string.h>
#include <base/stdint.h>

/* TODO:
 *   - use unsigned lengths
 *   - refactor older code
 */


unsigned strlen( char *input ){
	int i;
	for ( i = 0; input[i] != '\0'; i++ );
	return i;
}

int strcmp( char *s1, char *s2 ){
	unsigned i = 0, ret = 0, s1_len = 0, s2_len = 0;
	s1_len = strlen( s1 );
	s2_len = strlen( s2 );
	
	if ( s1_len != s2_len ){
		return s1_len - s2_len;
	}
	for ( i = 0; i < s1_len; i++ ){
		ret += (s1[i] == s2[i])?0:1;
	}
	return ret;
}

int strncmp( char *s1, char *s2, int len ){
	int ret = 0;
	int i;

	for ( i = 0; i < len; i++ ){
		ret += s1[i] != s2[i];

		if ( !s1[i] || !s2[i] )
			break;
	}

	return ret;
}

char *strdup( char *s ){
	char *ret = kmalloc( strlen( s ) + 1 );
	return memcpy( ret, s, strlen( s ) + 1 );
}

char *strcpy( char *dest, const char *src ){
	char *ret = dest;
	int i;

	for ( i = 0; src[i]; i++ )
		dest[i] = src[i];

	dest[i] = 0;

	return ret;
}

char *strncpy( char *dest, const char *src, unsigned len ){
	char *ret = dest;
	int i;

	for ( i = 0; src[i] && i < len; i++ )
		dest[i] = src[i];

	dest[i] = 0;

	return ret;
}

char *strcat( char *dest, const char *src ){
	char *ret = dest;
	int i;

	for ( i = 0; dest[i]; i++ );
	strcpy( dest + i, src );

	return ret;
}

char *strncat( char *dest, const char *src, int n ){
	char *ret = dest;

	int i;

	for ( i = 0; dest[i]; i++ );
	strncpy( dest + i, src, n );

	return ret;
}

/* Basic memory stuff */
void *memset( void *dest, unsigned char value, unsigned count ){
	char *ret_dest = dest;
	while ( count-- ){
		*(ret_dest++) = value;
	}
	return dest;
}

void *memsetw( void *dest, unsigned char value, unsigned count ){
	char *ret_dest = dest;
	while ( count-- ){
		*(ret_dest++) = value;
	}
	return dest;
}

void *memcpy( void *dest, void *src, unsigned count ){
	char *ret_dest = dest;
	char *src_dest = src;
	while ( count-- ){
		*(ret_dest++) = *(src_dest++);
	}
	return dest;
}

void *memmove( void *dest, void *src, unsigned count ){
	char *ret_dest = dest;
	char *src_dest = src;
	while ( count-- ){
		*(ret_dest++) = *(src_dest++);
		*(src_dest) = 0;
	}
	return dest;
}

int memcmp( void *s1, void *s2, int len ){
	int ret = 0;
	int i;

	char *foo = s1;
	char *bar = s2;

	for ( i = 0; i < len; i++ ){
		ret += foo[i] != bar[i];
	}

	return ret;
}

#endif
