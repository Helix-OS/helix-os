#include <base/datastructs/dlist.h>
#include <base/mem/alloc.h>
#include <base/string.h>

dlist_container_t *dlist_create( unsigned block_size, unsigned prealloc ){
	dlist_container_t *ret;

	ret = kmalloc( sizeof( dlist_container_t ));
	memset( ret, 0, sizeof( dlist_container_t ));

	ret->block_size = block_size? block_size : DLIST_DEFAULT_BLOCKSIZE;
	ret->blocks_alloced = prealloc? prealloc : 1;

	ret->alloced = ret->block_size * ret->blocks_alloced;
	ret->entries = kmalloc( sizeof( void *[ ret->alloced ]));

	return ret;
}

void dlist_free( dlist_container_t *list ){
	kfree( list->entries );
	kfree( list );
}

int dlist_add( dlist_container_t *list, void *data ){
	int i, j;

	for ( i = 0; i < list->alloced && list->entries[i]; i++ );

	if ( i >= list->alloced - 1 ){
		list->entries = krealloc( list->entries, (list->blocks_alloced + 1) * list->block_size * sizeof( void * ));

		list->alloced += list->block_size;
		list->blocks_alloced++;

		for ( j = i; j < list->alloced; j++ )
			list->entries[j] = 0;
	}
	
	list->entries[i] = data;
	list->used++;

	return i;
}

int dlist_remove( dlist_container_t *list, int index ){
	int ret = 0;

	if ( index < list->alloced && list->entries[index] ){
		list->entries[index] = 0;
		list->used--;
	}

	return ret;
}

void *dlist_get( dlist_container_t *list, int index ){
	void *ret = 0;

	if ( index < list->alloced && list->entries[index] )
		ret = list->entries[index];

	return ret;
}

// TODO: Realloc if index is greater than alloced
void *dlist_set( dlist_container_t *list, int index, void *data ){
	void *ret = 0;

	if ( index < list->alloced /* && list->entries[index] */ )
		ret = list->entries[index] = data;

	return ret;
}

unsigned dlist_allocated( dlist_container_t *list ){
	return list->alloced;
}

unsigned dlist_used( dlist_container_t *list ){
	return list->used;
}
