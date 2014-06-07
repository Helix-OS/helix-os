#include <base/initrd.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>
#include <base/logger.h>
#include <base/string.h>

static unsigned int octal_to_dec( int oct ){
	unsigned ret, i, k;

	for ( ret = 0, i = 1, k = oct; k; k /= 10, i *= 8 )
		ret += (k % 10) * i;

	return ret;
}

initrd_t *init_initrd( tar_header_t *header ){
	tar_header_t *move;
	initrd_t *ret = 0;
	unsigned size;
	int meh;
	
	if ( header ){
		ret = knew( initrd_t );
		ret->firsthead = header;
		ret->headers = dlist_create( 0, 0 );

		move = header;
		while ( *move->filename ){
			size = octal_to_dec( atoi( move->size ));
			meh = dlist_add( ret->headers, move );

			kprintf( "[%s] Have file \"%s\" at index %d\n", __func__, move->filename, meh );

			move += size / sizeof( tar_header_t ) + 1;
			move += size % sizeof( tar_header_t ) > 1;
		}
	}

	return ret;
}

tar_header_t *initrd_get( initrd_t *rd, int index ){
	return dlist_get( rd->headers, index );
}

tar_header_t *initrd_get_file( initrd_t *rd, char *name ){
	tar_header_t *ret = 0,
		     *move;
	int i;

	for ( i = 0; i < dlist_allocated( rd->headers ); i++ ){
		move = initrd_get( rd, i );
		if ( strcmp( move->filename, name ) == 0 ){
			ret = move;
			break;
		}
	}

	return ret;
}

unsigned initrd_get_size( tar_header_t *header ){
	return octal_to_dec( atoi( header->size ));
}
