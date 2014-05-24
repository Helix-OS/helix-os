#include <base/datastructs/list.h>
#include <base/mem/alloc.h>
#include <base/string.h>

list_head_t *list_create( int flags ){
	list_head_t *ret = 0;
	ret = kmalloc( sizeof( list_head_t ));
	memset( ret, 0, sizeof( list_head_t ));
	ret->flags = flags;

	return ret;
}

list_node_t *list_add_int( list_head_t *list, int val ){
	list_node_t *ret;

	ret = list->last = list_add_int_node( list->last, val );
	if ( !list->base )
		list->base = list->last;
	
	return ret;
}

list_node_t *list_add_data( list_head_t *list, void *data ){
	list_node_t *ret;

	ret = list->last = list_add_data_node( list->last, data );
	if ( !list->base )
		list->base = list->last;
	
	return ret;
}

list_node_t *list_remove_index( list_head_t *list, int index ){
	list_node_t *ret = 0,
		    *move;

	move = list_get_index( list, index );
	if ( move ){
		ret = list_remove_node( move );
	}

	return ret;
}

list_node_t *list_get_index( list_head_t *list, int i ){
	list_node_t *ret = list->base,
		    *move = 0;
	int n = 0;

	if ( i > 0 )
		move = list->base;
	else if ( i < 0 )
		move = list->last;

	while ( move && n != i ){
		if ( i > 0 ){
			n++;
			ret = move = move->next;
		} else if ( i < 0 ){
			n--;
			ret = move = move->prev;
		}
	}

	return ret;
}

list_node_t *list_get_val( list_head_t *list, int val ){
	list_node_t *ret = 0,
		    *move;

	move = list->base;
	foreach_in_list( move ){
		if ( move->val == val ){
			ret = move;
			break;
		}
	}

	return ret;
}

list_node_t *list_add_node( list_node_t *list, int val, void *data ){
	list_node_t *ret = 0,
		    *temp,
		    *move;

	temp = kmalloc( sizeof( list_node_t ));
	temp->val = val;
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

list_node_t *list_add_int_node( list_node_t *list, int val ){
	return list_add_node( list, val, 0 );
}

list_node_t *list_add_data_node( list_node_t *list, void *data ){
	return list_add_node( list, 0, data );
}

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

void list_free_nodes( list_node_t *node ){
	list_node_t *move,
		    *temp;

	for ( move = node; move->prev; move = move->prev );
	for ( ; move; move = temp ){
		temp = move->next;
		kfree( move );
	}
}

