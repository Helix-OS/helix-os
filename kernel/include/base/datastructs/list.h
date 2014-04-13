#ifndef _helix_datastruct_list
#define _helix_datastruct_list

#define foreach_in_list( list ) for ( ; list; list = list->next )

typedef struct list_node {
	struct list_node *next;
	struct list_node *prev;

	int val;
	void *data;
} list_node_t;

typedef struct list_head {
	struct list_node *base;
	struct list_node *last;

	unsigned length;
	unsigned flags;
} list_head_t;

// General purpose list functions
list_head_t *list_create( int flags );
list_node_t *list_add_int( list_head_t *list, int val );
list_node_t *list_add_data( list_head_t *list, void *data );
list_node_t *list_remove_index( list_head_t *list, int index );
list_node_t *list_get_index( list_head_t *list, int i );

// List node functions, intended for use by the list datastructure
list_node_t *list_add_int_node( list_node_t *list, int val );
list_node_t *list_add_data_node( list_node_t *list, void *data );
list_node_t *list_remove_node( list_node_t *node );

unsigned listlen( list_node_t *node );
void list_free_nodes( list_node_t *node );
void list_free( list_head_t *list );

#endif
