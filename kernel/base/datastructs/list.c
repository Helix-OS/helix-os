#include <base/datastructs/list.h>
#include <base/mem/alloc.h>

list_node_t *list_add_int_node( list_node_t *list, int val ){
	list_node_t *ret = 0,
		    *temp,
		    *move;

	temp = kmalloc( sizeof( list_node_t ));
	temp->val = val;
	temp->data = 0;
	ret = temp;

	if ( list ){
		for ( move = list; move->next; move = move->next );

		move->next = temp;
		move->next->prev = move;
		move->next->next = 0;
	} else {
		move = temp;
		move->next = 0;
		move->prev = 0;
	}

	return ret;
}

list_node_t *list_add_data_node( list_node_t *list, void *data ){
	list_node_t *ret = 0,
		    *temp,
		    *move;

	temp = kmalloc( sizeof( list_node_t ));
	temp->val = 0;
	temp->data = data;
	ret = temp;

	if ( list ){
		for ( move = list; move->next; move = move->next );

		move->next = temp;
		move->next->prev = move;
		move->next->next = 0;
	} else {
		move = temp;
		move->next = 0;
		move->prev = 0;
	}

	return ret;
}

// type-generic list_add_node defined in include/base/datastructs/list.h

list_node_t *list_remove_node( list_node_t *node ){
	list_node_t *ret = 0;

	if ( node ){
		if ( node->prev ){
			node->prev->next = node->next;
			ret = node->prev;
		}

		if ( node->next ){
			node->next->prev = node->prev;
			ret = ret? ret : node->next;
		}

		kfree( node );
	}

	return ret;
}

unsigned listlen( list_node_t *node ){
	list_node_t *move = node;
	unsigned i;

	for ( i = 0; move; move = move->next )
		i++;

	return i;
}

list_node_t *list_get_index( list_node_t *node, int i ){
	list_node_t *ret = node,
		    *move = node;
	int n = 0;

	while ( move && n != i ){
		if ( i > 0 ){
			i++;
			ret = move = move->next;
		} else if ( i < 0 ){
			i--;
			ret = move = move->prev;
		}
	}

	return ret;
}
