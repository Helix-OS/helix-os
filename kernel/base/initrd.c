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

/** \brief Initializes an \ref initrd_t struct from a pointer to a tar archive in memory.
 *  @param header Pointer to a tar archive entirely in memory.
 *  @return An \ref initrd_t struct representing the archive, for quick lookups.
 */
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

/** \brief Gets a given tar header from an initrd_t struct by index.
 *  @param rd    initrd_t struct returned from \ref init_initrd.
 *  @param index Which file to get from the archive, starting from 0
 *  @return If the file exists, the tar_header_t for the index given, otherwise it returns 0.
 */

tar_header_t *initrd_get( initrd_t *rd, int index ){
	return dlist_get( rd->headers, index );
}

/** \brief Gets a tar header from an initrd_t struct by name.
 *  @param rd    initrd_t struct returned from \ref init_initrd.
 *  @param name  The filename to find in the archive.
 *  @return If the file exists, the tar_header_t for the name given, otherwise it returns 0.
 */
tar_header_t *initrd_get_file( initrd_t *rd, char *name ){
	tar_header_t *ret = 0,
		     *move;
	int i;

	for ( i = 0; i < dlist_used( rd->headers ); i++ ){
		move = initrd_get( rd, i );

		if ( strcmp( move->filename, name ) == 0 ){
			ret = move;
			break;
		}
	}

	return ret;
}


/** \brief Gets the size of the file a header points to.
 *
 *  This is needed for cleanliness, since tar uses ascii strings for numerical fields.
 *
 *  @param header The header to get the size of.
 *  @return The size of the file.
 */
unsigned initrd_get_size( tar_header_t *header ){
	return octal_to_dec( atoi( header->size ));
}
