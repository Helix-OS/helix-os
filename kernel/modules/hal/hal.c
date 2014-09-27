#include <base/hal.h>
#include <base/logger.h>

char *depends[] = { "base", 0 };
char *provides = "hal";

static list_head_t *hal_device_list = 0;
static protected_var_t *p_hal_device_list = 0;

static list_head_t *hal_event_queue = 0;
static protected_var_t *p_hal_event_queue = 0;

int hal_register_device( hal_device_t *device ){
	list_head_t *device_list;

	if ( !device )
		return -1;

	device_list = access_protected_var( p_hal_device_list );
	list_add_data( device_list, device );
	leave_protected_var( p_hal_device_list );

	return 0;
}

int hal_unregister_device( hal_device_t *device ){
	if ( !device )
		return -1;

	/* TODO: Fill this in */

	return 0;
}

list_head_t *hal_get_device_list( ){
	return hal_device_list;
}

hal_device_t *hal_get_device( unsigned devnum ){
	hal_device_t *ret = 0;
	list_head_t *device_list;
	list_node_t *devnode;

	device_list = access_protected_var( p_hal_device_list );

	devnode = list_get_index( device_list, devnum );
	if ( devnode )
		ret = devnode->data;

	leave_protected_var( p_hal_device_list );

	return ret;
}

void hal_dump_devices( ){
	hal_device_t *move;
	list_head_t *device_list;
	list_node_t *node;

	device_list = access_protected_var( p_hal_device_list );

	node = list_get_index( device_list, 0 );
	foreach_in_list( node ){
		move = node->data;

		kprintf( "[hal] 0x%x: type = 0x%x, flags = 0x%x\n",
				move, move->type, move->flags );
	}

	leave_protected_var( p_hal_device_list );
}

//void init_hal( ){
int init( ){
	hal_device_list = list_create( 0 );
	p_hal_device_list = create_protected_var( 1, hal_device_list );

	hal_event_queue = list_create( 0 );
	p_hal_event_queue = create_protected_var( 1, hal_event_queue );

	kprintf( "[%s] Hello world!\n", provides );
	kprintf( "[%s] This is module \"%s\", and I'm in yo kernelz\n",
			provides, provides );

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
