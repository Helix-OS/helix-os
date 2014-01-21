#ifndef _helix_datastr_list
#define _helix_datastr_list

// Generic macro, depends on c11 support
#define list_add_node( list, N )\
	_Generic((N), 	int:    list_add_int_node,\
			void *: list_add_data_node )( list, N )

typedef struct list_node {
	struct list_node *next;
	struct list_node *prev;

	int val;
	void *data;
} list_node_t;

#endif
