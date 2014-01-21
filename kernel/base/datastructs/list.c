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
