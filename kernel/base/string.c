#ifndef _kernel_string_c
#define _kernel_string_c
#include <base/string.h>
#include <base/stdint.h>

/* TODO: - use unsigned lengths */

unsigned strlen( const char *input ){
	int i;

	for ( i = 0; input[i] != '\0'; i++ );

	return i;
}

int strcmp( const char *s1, const char *s2 ){
	int ret = 0;
	unsigned i;

	for ( i = 0;; i++ ){
		ret += s1[i] != s2[i];

		if ( !s1[i] || !s2[i] )
			break;
	}

	return ret;
}

int strncmp( const char *s1, const char *s2, int len ){
	int ret = 0;
	int i;

	for ( i = 0; i < len; i++ ){
		ret += s1[i] != s2[i];

		if ( !s1[i] || !s2[i] )
			break;
	}

	return ret;
}

char *strdup( const char *s ){
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
	uint8_t *ret_dest = dest;

	while ( count-- ){
		*(ret_dest++) = value;
	}

	return dest;
}

void *memsetw( void *dest, unsigned char value, unsigned count ){
	uint8_t *ret_dest = dest;

	while ( count-- ){
		*(ret_dest++) = value;
	}

	return dest;
}

void *memcpy( void *dest, const void *src, unsigned count ){
	uint8_t *byte_ret;
	const uint8_t *byte_src;

	uint32_t *long_ret = dest;
	const uint32_t *long_src = src;

	while ( count > 4 ){
		*(long_ret++) = *(long_src++);
		count -= 4;
	}

	byte_ret = (uint8_t *)long_ret;
	byte_src = (const uint8_t *)long_src;

	while ( count ){
		*(byte_ret++) = *(byte_src++);
		count--;
	}

	return dest;
}

void *memmove( void *dest, const void *src, unsigned count ){
	uint8_t *ret_dest = dest;
	const uint8_t *src_dest = src;

	while ( count-- ){
		*(ret_dest++) = *(src_dest++);
	}

	return dest;
}

int memcmp( const void *s1, const void *s2, unsigned len ){
	int ret = 0;
	unsigned i;

	const uint8_t *foo = s1;
	const uint8_t *bar = s2;

	for ( i = 0; i < len; i++ ){
		ret += foo[i] != bar[i];
	}

	return ret;
}

#endif
