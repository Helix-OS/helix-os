#ifndef _helix_hal_h
#define _helix_hal_h
#include <base/string.h>
#include <base/datastructs/list.h>
#include <base/tasking/semaphore.h>

enum {
	HAL_TYPE_NULL,
	HAL_TYPE_GENERAL,
	HAL_TYPE_STORAGE,
	HAL_TYPE_VIDEO,
};

enum {
	HAL_FLAG_NULL,
};

struct hal_device;
struct hal_device_info;

typedef int (*hal_device_read_block)( struct hal_device *dev, void *buf,
					unsigned count, unsigned offset );
typedef int (*hal_device_write_block)( struct hal_device *dev, void *buf,
					unsigned count, unsigned offset );

typedef struct hal_device {
	hal_device_read_block 	read;
	hal_device_write_block 	write;
	void 	*dev;

	unsigned block_size;
	unsigned type;
	unsigned flags;
} hal_device_t;

int hal_register_device( hal_device_t *device );
int hal_unregister_device( hal_device_t *device );
hal_device_t *hal_get_device( unsigned devnum );
list_head_t *hal_get_device_list( );

void hal_dump_devices( );
void init_hal( );

#endif
