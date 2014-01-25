#ifndef _helix_datastruct_list
#define _helix_datastruct_list

#define foreach_in_list( list ) for ( ; list; list = list->next )

typedef struct list_node {
	struct list_node *next;
	struct list_node *prev;

	int val;
	void *data;
} list_node_t;

list_node_t *list_add_int_node( list_node_t *list, int val );
list_node_t *list_add_data_node( list_node_t *list, void *data );
list_node_t *list_remove_node( list_node_t *node );

list_node_t *list_get_index( list_node_t *node, int i );
unsigned listlen( list_node_t *node );
void list_free( list_node_t *node );

#endif
