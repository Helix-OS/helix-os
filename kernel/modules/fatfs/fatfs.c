#include <fatfs/fatfs.h>
#include <base/logger.h>
#include <base/kstd.h>
#include <base/string.h>

char *depends[] = { "base", 0 };
char *provides = "fatfs";

static file_funcs_t fatfs_functions = {
	.lookup = 0,
};

struct file_system *fatfs_create( struct file_driver *device, struct file_system *unused,
				char *path, unsigned flags )
{
	file_system_t *ret = knew( file_system_t );
	fatfs_device_t *dev = ret->devstruct = knew( fatfs_device_t );
	dev->bpb = knew( uint8_t[512] );

	if ( file_lookup_absolute( path, &dev->device_node, 0 ) >= 0 ){

		kprintf( "[%s] Woot, got here\n", __func__ );
		VFS_FUNCTION(( &dev->device_node ), read, dev->bpb, 512, 0 );

		{ 	int i;
			char *buf = (char *)dev->bpb;

			for ( i = 0; i < 512; i++ ){
				kprintf( "%c", buf[i] );
			}
		}

		kprintf( "\n" );

		kprintf( "[%s] File system \"%s\" has %d FATs\n", __func__, dev->bpb->oem_ident, dev->bpb->fats );

		// Obviously remove this once actual code is written
		kfree( dev->bpb );
		kfree( dev );
		kfree( ret );
		ret = 0;

	} else {
		kfree( ret );
		ret = 0;
	}

	return ret;
}

int test( ){
	file_node_t fnode;
	file_mount_filesystem( "/test/fatdir", "/test/devices/device1", "fatfs", 0 );

	return 0;
}

int init( ){
	file_driver_t *fatfs_driver;

	kprintf( "[%s] Hello world!\n", provides );
	kprintf( "[%s] This is module \"%s\", and I'm in yo kernelz\n",
			provides, provides );

	fatfs_driver = knew( file_driver_t );
	fatfs_driver->name = strdup( "fatfs" );
	fatfs_driver->create = fatfs_create;

	file_register_driver( fatfs_driver );
	test( );
	
	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
