#ifndef _helix_datastruct_dynstring_h
#define _helix_datastruct_dynstring_h
#include <base/string.h>

#define DEFAULT_DYNSTRING_BSIZE 32

typedef struct dynstring {
	char *string;

	unsigned length;
	unsigned alloced;
	unsigned block_size;
	unsigned references;
} dynstring_t;

dynstring_t *dstring_create( char *string );
dynstring_t *dstring_copy( dynstring_t *string );
void dstring_destroy( dynstring_t *string );

dynstring_t *dstring_set_char( dynstring_t *string,
		unsigned pos, char newchar );

char *dstring_get_string( dynstring_t *string );
dynstring_t *dstring_set_string( dynstring_t *string, char *newstr );

#endif
