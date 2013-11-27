#ifndef _kernel_string_c
#define _kernel_string_c
#include <base/string.h>

unsigned int strlen( char *input ){
	int i;
	for ( i = 0; input[i] != '\0'; i++ );
	return i;
}

int strcmp( char *s1, char *s2 ){
	unsigned int i = 0, ret = 0, s1_len = 0, s2_len = 0;
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

/* Basic memory stuff */
void *memset( void *dest, unsigned char value, unsigned int count ){
	char *ret_dest = dest;
	while ( count-- ){
		*(ret_dest++) = value;
	}
	return dest;
}

void *memsetw( void *dest, unsigned char value, unsigned int count ){
	char *ret_dest = dest;
	while ( count-- ){
		*(ret_dest++) = value;
	}
	return dest;
}

void *memcpy( void *dest, void *src, unsigned int count ){
	char *ret_dest = dest;
	char *src_dest = src;
	while ( count-- ){
		*(ret_dest++) = *(src_dest++);
	}
	return dest;
}

void *memmove( void *dest, void *src, unsigned int count ){
	char *ret_dest = dest;
	char *src_dest = src;
	while ( count-- ){
		*(ret_dest++) = *(src_dest++);
		*(src_dest) = 0;
	}
	return dest;
}

#endif
