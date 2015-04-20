#include <base/logger.h>
#include <base/hal.h>
#include <base/stdint.h>

typedef struct mbr_part_entry {
	uint8_t flags;
	uint8_t unused1[3];
	uint8_t system_id;
	uint8_t unused2[3];
	uint32_t rel_sect;
	uint32_t total_sect;
} __attribute__((packed)) mbr_part_entry_t;

typedef struct part_dev {
	hal_device_t *orig_device;
	mbr_part_entry_t entry;
} part_dev_t;

char *depends[] = { "base", "hal", 0 };
char *provides = "parts";

static int partition_read( struct hal_device *dev, void *buf, unsigned count, unsigned offset );
static int partition_write( struct hal_device *dev, void *buf, unsigned count, unsigned offset );

void check_for_parts( ){
	list_head_t *devlist;
	list_node_t *move;
	list_node_t *last;
	hal_device_t *dev, *temp;
	part_dev_t *pdev;
	char *buf = knew( char[512] );
	mbr_part_entry_t *parts = (void *)(buf + 0x1be);
	int i;

	devlist = hal_get_device_list( );
	move = devlist->base;
	last = devlist->last;

	foreach_in_list( move ){
		dev = move->data;

		if ( dev->type == HAL_TYPE_STORAGE ){
			kprintf( "[%s] Have storage device \"%s\", "
					 "checking for MBR-style partitions\n",
					__func__, dev->name );

			memset( buf, 0, 512 );
			dev->read( dev, buf, 1, 0 );

			for ( i = 0; i < 4; i++ ){
				if ( parts[i].system_id != 0 ){
					kprintf( "[%s] %sp%d: 0x%x, %d sectors, starting at %d\n",
						__func__, dev->name, i, parts[i].flags, parts[i].total_sect, parts[i].rel_sect );

					temp = knew( hal_device_t );
					temp->dev = pdev = knew( part_dev_t );

					pdev->orig_device = dev;
					memcpy( &pdev->entry, parts + i, sizeof( mbr_part_entry_t ));

					temp->read = partition_read;
					temp->write = partition_write;

					temp->block_size = dev->block_size;
					temp->type = dev->type;
					temp->subtype = dev->subtype;

					temp->name = knew( char[ strlen( dev->name ) + 8 ]);
					strcpy( temp->name, dev->name );
					strcat( temp->name, (char []){ 'p', '0' + i, 0 });

					hal_register_device( temp );

					kprintf( "[%s] %s registered\n", __func__, temp->name );
				}
			}
		}

		// XXX: prevent the scanner from looking at the partitions that were just added for partitions
		if ( move == last )
			break;
	}

	memset( buf, 0, 512 );
	kfree( buf );
}

static int partition_read( struct hal_device *dev, void *buf, unsigned count, unsigned offset ){
	int ret = 0;
	part_dev_t *pdev;

	if ( dev ){
		pdev = dev->dev;

		if ( pdev ){
			ret = pdev->orig_device->read( pdev->orig_device, buf, count, pdev->entry.rel_sect + offset );
		}
	}

	return ret;
}

static int partition_write( struct hal_device *dev, void *buf, unsigned count, unsigned offset ){
	int ret = 0;
	part_dev_t *pdev;

	if ( dev ){
		pdev = dev->dev;

		if ( pdev ){
			ret = pdev->orig_device->write( pdev->orig_device, buf, count, pdev->entry.rel_sect + offset );
		}
	}

	return ret;
}

int init( ){
	kprintf( "[%s] Hello world!\n", provides );
	kprintf( "[%s] This is module \"%s\", and I'm in yo kernelz\n",
			provides, provides );

	hal_dump_devices( );
	check_for_parts( );

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
