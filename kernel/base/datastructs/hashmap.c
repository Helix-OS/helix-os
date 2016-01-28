#include <base/datastructs/hashmap.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>

hashmap_t *hashmap_create( unsigned n ){
	hashmap_t *ret;

	ret = knew( hashmap_t );
	ret->buckets = knew( list_head_t[n] );
	ret->nbuckets = n;

	return ret;
}

void hashmap_free( hashmap_t *map ){
	if ( map ){
		kfree( map->buckets );
		kfree( map );
	}
}

void *hashmap_add( hashmap_t *map, unsigned hash, void *val ){
	list_head_t *list;
	list_node_t *node;
	void *ret = 0;
	
	list = map->buckets + ( hash % map->nbuckets );

	node = list_add_data( list, val );
	if ( node ){
		node->val = hash;
		ret = node->data;
	}

	return ret;
}

void *hashmap_get( hashmap_t *map, unsigned hash ){
	list_head_t *list;
	list_node_t *node;
	void *ret = 0;
	
	list = map->buckets + ( hash % map->nbuckets );
	node = list_get_val( list, hash );

	if ( node )
		ret = node->data;

	return ret;
}

void hashmap_remove( hashmap_t *map, unsigned hash ){
	list_head_t *list;
	
	list = map->buckets + ( hash % map->nbuckets );
	list_remove_val( list, hash );
}
