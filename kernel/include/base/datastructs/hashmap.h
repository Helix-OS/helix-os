#ifndef _helix_datastruct_hashmap
#define _helix_datastruct_hashmap
#include <base/datastructs/list.h>

typedef struct hashmap {
	list_head_t	*buckets;
	unsigned 	nbuckets;
} hashmap_t;

hashmap_t *hashmap_create( unsigned n );
void hashmap_free( hashmap_t *map );

void *hashmap_add( hashmap_t *map, unsigned hash, void *val );
void *hashmap_get( hashmap_t *map, unsigned hash );
void hashmap_remove( hashmap_t *map, unsigned hash );

#endif
