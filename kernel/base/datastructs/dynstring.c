#include <base/datastructs/dynstring.h>
#include <base/mem/alloc.h>
#include <base/kstd.h>

dynstring_t *dstring_create( char *string ){
	dynstring_t *ret;

	ret = knew( dynstring_t );

	ret->string = 0;
	dstring_set_string( ret, string );
	ret->references = 0;

	return ret;
}

dynstring_t *dstring_copy( dynstring_t *string ){
	dynstring_t *ret;

	ret = dstring_create( dstring_get_string( string ));

	return ret;
}

void dstring_destroy( dynstring_t *string ){
	kfree( string->string );
	kfree( string );
}

dynstring_t *dstring_set_char( dynstring_t *string, unsigned pos, char newchar ){
	dynstring_t *ret = string;

	if ( pos > string->length && pos > ( string->alloced * string->block_size )){
		string->alloced = pos / string->block_size +
				( pos % string->block_size );
		
		string->string = krealloc( string->string, string->alloced * string->block_size );

	} else if ( pos > string->length ){
		string->length = pos;

	}

	string->string[pos] = newchar;

	return ret;
}

char *dstring_get_string( dynstring_t *string ){
	char *ret;

	ret = string->string;

	return ret;
}

dynstring_t *dstring_set_string( dynstring_t *string, char *newstr ){
	dynstring_t *ret = string;

	if ( string->string )
		kfree( string->string );

	string->string = strdup( newstr );
	string->length = strlen( newstr );

	string->block_size = DEFAULT_DYNSTRING_BSIZE;
	string->alloced = string->length / DEFAULT_DYNSTRING_BSIZE +
				( string->length % DEFAULT_DYNSTRING_BSIZE )? 1: 0;

	return ret;
}
