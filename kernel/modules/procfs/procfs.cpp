#include <base/logger.h>
#include <base/string.h>
#include <base/tasking/task.h>
#include <base/vfs/vfs.h>
#include <cpplib/cpplib.h>
#include <cpplib/memory.h>

extern "C" {
	const char *depends[] = { "cpplib", "base", 0 };
	const char *provides = "procfs";

	int  init( );
	void remove( );
}

using namespace klib;

// global file system data for the module
static file_system_t *procfs = nullptr;
// global root node for the filesystem
static file_node_t *fs_root = nullptr;

static int procfs_read( file_node_t *node, void *buffer, size_t length, size_t offset );
static int procfs_get_info( file_node_t *node, file_info_t *buf );
static int procfs_lookup( file_node_t *node, file_node_t *buf, char *name, int flags );
static int procfs_open( file_node_t *node, char *path, int flags );
static int procfs_close( file_node_t *node, int flags );
static int procfs_readdir( file_node_t *node, struct dirent *dirp, int entry );

file_funcs_t functions = {
	.get_info = procfs_get_info,
	.open     = procfs_open,
	.close    = procfs_close,
	.read     = procfs_read,
	.write    = nullptr,
	.mkdir    = nullptr,
	.mknod    = nullptr,
	.mount    = nullptr,
	.link     = nullptr,
	.unlink   = nullptr,
	.readdir  = procfs_readdir,
	.lookup   = procfs_lookup,
	.poll     = nullptr,
};

static file_system_t *create_procfs( struct file_driver *driver,
		struct file_system *unused, char *path, unsigned flags ){

	procfs->references++;

	return procfs; 
}

static int remove_procfs( struct file_driver *driver, struct file_system *fs, unsigned flags ){
	int ret = 0;

	if ( fs->references ){
		fs->references--;
	} else {
		ret = -ERROR_INVALID_ARGUMENT;
		kprintf( "[%s] Have double removal of procfs\n", __func__ );
	}

	return ret;
}

static int procfs_read( file_node_t *node, void *buffer, size_t length, size_t offset ){
	return -ERROR_NO_FUNC;
}

static int procfs_get_info( file_node_t *node, file_info_t *buf ){
	return -ERROR_NO_FUNC;
}

enum {
	INODE_SYSTEM  = 0x10000000,
	INODE_PROCESS = 0xcafe0000,
	INODE_MASK    = 0xffff0000,
};

static int procfs_lookup( file_node_t *node, file_node_t *buf, char *name, int flags ){
	int ret = -ERROR_NOT_FOUND;
	bool is_pid; 
	char *s;

	//pid = atoi( name );
	for ( s = name; *s; s++ ){
		if ( *s <= '0' || *s >= '9' ){
			is_pid = false;
			break;
		}
	}

	if ( node->inode == (INODE_SYSTEM | 1)){
		if ( strcmp( name, "threads" ) == 0 ){
			buf->inode = INODE_SYSTEM | 2;

		} else if ( strcmp( name, "system" ) == 0 ){
			buf->inode = INODE_SYSTEM | 3;
		}

	} else if ( node->inode == (INODE_SYSTEM | 2)){
		if ( is_pid ){
			unsigned pid = atoi( name );

			if ( get_task_node_by_pid( pid )){
				buf->inode = (0xcafe << 16) | pid;

			} else {
				ret = -ERROR_NOT_FOUND;
			}
		}
	}

	return ret;
}

static int procfs_open( file_node_t *node, char *path, int flags ){
	return -ERROR_NO_FUNC;
}

static int procfs_close( file_node_t *node, int flags ){
	return -ERROR_NO_FUNC;
}

static int procfs_readdir( file_node_t *node, struct dirent *dirp, int entry ){
	return -ERROR_NO_FUNC;
}

int init( ){
	file_driver_t *new_driver = new file_driver_t;
	procfs = new file_system_t;
	procfs->root_node = fs_root = new file_node_t;

	new_driver->name   = strdup( "procfs" );
	new_driver->create = create_procfs;
	new_driver->remove = remove_procfs;
	new_driver->flags  = 0;
	new_driver->references = 0;

	fs_root->inode      = INODE_SYSTEM | 1;
	fs_root->fs         = procfs;
	fs_root->references = 1;
	fs_root->mount      = 0;

	file_register_driver( new_driver );

	kprintf( "[%s] Hello world, this is %s\n", __func__, provides );

	return 0;
}

void remove( ){

}
