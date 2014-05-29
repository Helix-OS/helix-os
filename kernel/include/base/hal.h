#ifndef _helix_hal_h
#define _helix_hal_h
#include <base/string.h>
#include <base/datastructs/list.h>
#include <base/tasking/semaphore.h>

/* Main hardware abstraction layer types */
enum {
	HAL_TYPE_NULL,
	HAL_TYPE_GENERAL,
	HAL_TYPE_STORAGE,
	HAL_TYPE_VIDEO,
	HAL_TYPE_PERIPHERAL,
};

/* General subtypes */
enum {
	HAL_GENERAL_NULL,
};

/* Storage subtypes */
enum {
	HAL_STORAGE_NULL,
	HAL_STORAGE_HARDDISK,
};

/* Video subtypes */
enum {
	HAL_VIDEO_NULL,
	HAL_VIDEO_TEXT_BUF,
	HAL_VIDEO_FRAME_BUF,
};

/* Peripheral subtypes */
enum {
	HAL_PERIPHERAL_NULL,
	HAL_PERIPHERAL_KEYBOARD,
	HAL_PERIPHERAL_MOUSE,
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

	char 	*name;
	unsigned type;
	unsigned subtype;

	unsigned block_size;
	unsigned flags;
} hal_device_t;

int hal_register_device( hal_device_t *device );
int hal_unregister_device( hal_device_t *device );
hal_device_t *hal_get_device( unsigned devnum );
list_head_t *hal_get_device_list( );

void hal_dump_devices( );
void init_hal( );

#endif
