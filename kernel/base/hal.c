#include <base/hal.h>

static hal_device_t *hal_device_list = 0;

int hal_register_device( hal_device_t *device ){
	hal_device_t *move;

	if ( !device )
		return -1;

	if ( hal_device_list ){
		for ( move = hal_device_list; move->next; move = move->next );
		move->next = device;
		device->next = 0;
		device->prev = move;
	} else {
		device->prev = 0;
		device->next = 0;
		hal_device_list = device;
	}

	return 0;
}

int hal_unregister_device( hal_device_t *device ){
	if ( !device )
		return -1;

	if ( device->prev )
		device->prev->next = device->next;

	if ( device->next )
		device->next->prev = device->prev;

	return 0;
}

hal_device_t *hal_get_device_list( ){
	return hal_device_list;
}

hal_device_t *hal_get_device( hal_device_t *dev, unsigned type ){
	hal_device_t *move,
		     *ret = 0;

	if ( !dev )
		return 0;

	for ( move = dev; move; move = move->next ){
		if ( move->type == type ){
			ret = move;
			break;
		}
	}

	return ret;
}

#include <base/logger.h>
void hal_dump_devices( ){
	hal_device_t *move;

	for ( move = hal_device_list; move; move = move->next )
		kprintf( "[hal] 0x%x: type = 0x%x, flags = 0x%x\n",
				move, move->type, move->flags );
}

