#include <devfs/devfs.h>
#include <base/kstd.h>
#include <base/string.h>
#include <base/hal.h>

char *depends[] = { "base", 0 };
char *provides = "devfs";

static int devfs_get_info_node( struct file_node *node, struct file_info *buf );
static int devfs_readdir_node( struct file_node *node, struct dirent *dirp, int entry );
static int devfs_lookup_node( struct file_node *node, struct file_node *buf, char *name, int flags );
static int devfs_read_node( struct file_node *node, void *buffer, unsigned long length, unsigned long offset );
static int devfs_open_node( struct file_node *node, char *path, int flags );
static int devfs_write_node( struct file_node *node, void *buffer, unsigned long length, unsigned long offset );

static file_system_t *devfs = 0;
static file_funcs_t devfs_functions = {
	.get_info 	= devfs_get_info_node,
	.lookup 	= devfs_lookup_node,

	.readdir 	= devfs_readdir_node,

	.open		= devfs_open_node,
	.read  		= devfs_read_node,
	.write 		= devfs_write_node,
};

static file_system_t *create_devfs( struct file_driver *driver,
		struct file_system *unused, char *path, unsigned flags ){

	file_system_t *ret;

	devfs->references++;
	ret = devfs;

	return ret;
}

static int remove_devfs( struct file_driver *driver, struct file_system *fs, unsigned flags ){
	int ret = 0;

	if ( fs->references )
		fs->references--;
	else
		ret = -ERROR_INVALID_ARGUMENT;

	return ret;
}

static int devfs_get_info_node( struct file_node *node, struct file_info *buf ){
	int ret = 0;

	if ( buf ){
		if ( node->inode < 128 )
			buf->type = FILE_TYPE_DIR;
		else
			buf->type = FILE_TYPE_REG;
	}

	buf->mask = 0777;
	buf->uid = buf->gid = 0;
	buf->inode = node->inode;
	buf->links = 1;
	buf->flags = 0;
	buf->dev_id = (node->inode > 127)? 0 : node->inode - 128;

	return ret;
}

static int devfs_lookup_node( struct file_node *node, struct file_node *buf, char *name, int flags ){
	dirent_t *dirbuf;
	int ret = -ERROR_NOT_FOUND,
	    i;

	dirbuf = knew( dirent_t );
		
	for ( i = 0; devfs_readdir_node( node, dirbuf, i ); i++ ){
		if ( strcmp( name, dirbuf->name ) == 0 ){
			buf->inode = dirbuf->inode;
			buf->references = 0;
			buf->flags = 0;

			buf->fs = devfs;
			buf->mount = 0;

			ret = 0;
			break;
		}
	}

	kfree( dirbuf );

	return ret;
}

static int devfs_readdir_node( struct file_node *node, struct dirent *dirp, int entry ){
	int ret = 0;
	hal_device_t *devbuf;

	if ( node->inode < 128 ){
		// TODO: Have different directories for different types of devices
		devbuf = hal_get_device( entry );
		if ( devbuf ){
			// TODO: Have the type of the device in the name
			strcpy( dirp->name, devbuf->name );
			dirp->inode = 128 + entry;

			ret = 1;
		}

	} else {
		ret = -ERROR_NOT_DIRECTORY;
	}

	return ret;
}

static int devfs_read_node( struct file_node *node, void *buffer,
		unsigned long length, unsigned long offset )
{
	int ret = -ERROR_INVALID_PATH;
	hal_device_t *devbuf;

	devbuf = hal_get_device( node->inode - 128 );

	if ( devbuf ){
		if ( devbuf->read ){
			kprintf( "[%s] Got here, devbuf: 0x%x, dev: 0x%x, read: 0x%x\n", __func__, devbuf, devbuf->dev, devbuf->read );
			// TODO: Properly handle reads not align on devbuf->block_size
			ret = devbuf->read( devbuf, buffer, length / devbuf->block_size, offset / devbuf->block_size );

		} else {
			ret = -ERROR_NO_FUNC;
		}
	}

	return ret;
}

static int devfs_write_node( struct file_node *node, void *buffer,
		unsigned long length, unsigned long offset )
{
	int ret = -ERROR_INVALID_PATH;
	hal_device_t *devbuf;

	devbuf = hal_get_device( node->inode - 128 );

	if ( devbuf ){
		if ( devbuf->write ){
			kprintf( "[%s] Got here, devbuf: 0x%x, dev: 0x%x, write: 0x%x\n", __func__, devbuf, devbuf->dev, devbuf->write );
			// TODO: Properly handle writes not aligned on devbuf->block_size
			ret = devbuf->write( devbuf, buffer, length / devbuf->block_size, offset / devbuf->block_size );

		} else {
			ret = -ERROR_NO_FUNC;
		}
	}

	return ret;
}

static int devfs_open_node( struct file_node *node, char *path, int flags ){
	int ret = -ERROR_NOT_FOUND;
	file_node_t meh;

	if ( file_lookup_relative( path, node, &meh, 0 ) == 0 )
		ret = meh.inode;

	return ret;
}

int test( ){
	file_node_t fnode;
	int ret = 0;
	/*
	dirent_t testdir;
	int i;

	char *blargbuf = knew( char[512] );
	*/

	file_mount_filesystem( "/test/devices", NULL, "devfs", 0 );
	file_lookup_absolute( "/test/devices", &fnode, 0 );
	
	/*
	for ( i = 0; VFS_FUNCTION(( &fnode ), readdir, &testdir, i ); i++ )
		kprintf( "[devfs.test] Have device file \"%s\" with inode %d\n", testdir.name, testdir.inode );

	file_lookup_absolute( "/test/devices/device0", &fnode, 0 );
	i = VFS_FUNCTION(( &fnode ), read, blargbuf, 512, 0 );

	kprintf( "[devfs.test] Read function returned %d\n", i );

	kprintf( "[devfs.test] Read test dump:\n" );

	for ( i = 0; i < 512; i++ )
		kprintf( "%c", blargbuf[i] );

	kprintf( "\n" );
	kfree( blargbuf );
	*/

	return ret;
}

int init( ){
	file_driver_t *new_driver;
	file_node_t *fsroot;

	new_driver = knew( file_driver_t );
	new_driver->name = strdup( "devfs" );
	new_driver->flags = FILE_DRIVER_ALLOW_NULL_PATH;
	new_driver->create = create_devfs;
	new_driver->remove = remove_devfs;

	devfs = knew( file_system_t );
	devfs->functions = &devfs_functions;

	devfs->root_node = fsroot = knew( file_node_t );
	fsroot->inode = 0;
	fsroot->references = 1;
	fsroot->fs = devfs;
	fsroot->mount = 0;

	file_register_driver( new_driver );
	test( );

	return 0;
}

void remove( ){

}
