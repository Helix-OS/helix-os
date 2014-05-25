#ifndef _helix_datastruct_dlist
#define _helix_datastruct_dlist

#define DLIST_DEFAULT_BLOCKSIZE 8

enum {
	DLIST_FLAG_NULL
};

// TODO: Rename this to "struct dlist" and "dlist_t"
typedef struct dlist_container {
	void 		**entries;
	unsigned 	used;
	unsigned 	alloced;
	unsigned 	block_size;
	unsigned 	blocks_alloced;

	unsigned 	max_alloc;
	unsigned 	flags;
} dlist_container_t;

dlist_container_t *dlist_create( unsigned block_size, unsigned prealloc );
void dlist_free( dlist_container_t *list );

int dlist_add( dlist_container_t *list, void *data );
int dlist_remove( dlist_container_t *list, int index );
void *dlist_get( dlist_container_t *list, int index );
void *dlist_set( dlist_container_t *list, int index, void *data );

unsigned dlist_allocated( dlist_container_t *list );
unsigned dlist_used( dlist_container_t *list );

#endif
