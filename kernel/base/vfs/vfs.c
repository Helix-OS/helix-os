#include <base/vfs/vfs.h>
#include <base/vfs/ramfs/ramfs.h>
#include <base/tasking/semaphore.h>
#include <base/datastructs/list.h>
#include <base/datastructs/hashmap.h>
#include <base/lib/stdbool.h>
#include <base/logger.h>
#include <base/string.h>
#include <base/syscalls.h>
#include <base/tasking/task.h>
#include <base/ipc/pipes.h>

char *provides = "vfs";

static list_node_t *driver_list = 0;
//static list_node_t *mount_list = 0;
static hashmap_t *mount_map;

static file_node_t *global_vfs_root;

static protected_var_t *p_driver_list;
//static protected_var_t *p_mount_list;
static protected_var_t *p_mount_map;

int file_register_driver( file_driver_t *driver ){
	list_node_t *drivers;

	drivers = access_protected_var( p_driver_list );

	// Replace first dummy node if it's still there
	if ( !drivers->data ){
		drivers->data = driver;

	} else {
		list_add_data_node( drivers, driver );
	}

	driver->references++;
	leave_protected_var( p_driver_list );

	return 0;
}

static inline unsigned vfs_node_hash( file_node_t *node ){
	return (node->fs->id << 24) | node->inode;
}

static inline unsigned vfs_alloc_fs_id( void ){
	static unsigned fs_count = 0x1;

	return fs_count++;
}

int file_register_mount( unsigned hash, file_node_t *mountnode ){
	//list_node_t *mounts;
	hashmap_t *mounts;
	//file_mount_t *new_mount;

	kprintf( "[%s] Adding mount node for hash 0x%x -> fs %u, inode %u\n",
		__func__, hash, mountnode->fs->id, mountnode->inode );
	mounts = access_protected_var( p_mount_map );
	hashmap_add( mounts, hash, mountnode );
	leave_protected_var( p_mount_map );
	//return new_mount;
	return 0;
}

file_node_t *file_get_mount( file_node_t *buf, unsigned hash ){
	hashmap_t *mounts;
	file_node_t *temp;
	file_node_t *ret = NULL;

	kprintf( "[%s] Looking for mount with hash 0x%x...\n", __func__, hash );

	mounts = access_protected_var( p_mount_map );
	temp = hashmap_get( mounts, hash );

	if ( temp ){
		kprintf( "[%s] Found hash 0x%x, copying to buffer\n", __func__, hash );
		memcpy( buf, temp, sizeof( file_node_t ));
		ret = buf;
	}

	leave_protected_var( p_mount_map );
	return ret;
}

file_driver_t *file_get_driver( char *name ){
	list_node_t *temp = driver_list;
	file_driver_t *ret = 0,
		      *test;

	foreach_in_list( temp ){
		test = temp->data;
		if ( strcmp( test->name, name ) == 0 ){
			ret = test;
			break;
		}
	}

	return ret;
}

bool file_is_same_fs( file_node_t *node, file_node_t *other ) {
	return node->fs->id == other->fs->id;
}

void set_global_vfs_root( file_node_t *root ){
	global_vfs_root = root;
}

file_node_t *get_global_vfs_root( ){
	return global_vfs_root;
}

file_node_t *get_local_vfs_root( ){
	task_t *cur = get_current_task( );
	file_node_t *ret = get_global_vfs_root( );

	if ( cur->froot )
		ret = cur->froot;

	return ret;
}

file_node_t *get_current_dir( ){
	task_t *cur = get_current_task( );
	file_node_t *ret = cur->curdir;

	if ( !ret )
		ret = get_local_vfs_root( );

	return ret;
}

int file_mount_filesystem( char *mount_path, char *device, char *filesystem, int flags ){
	file_driver_t *driver;
	file_system_t *fs;
	file_node_t *mount = 0;
	int ret = 0,
	    foo;

	driver = file_get_driver( filesystem );

	if ( !driver ){
		ret = -1;
		goto done;
	}

	fs = driver->create( driver, NULL, device, flags );
	if ( !fs ){
		ret = -1;
		goto done;
	}
	fs->id = vfs_alloc_fs_id( );

	//file_register_mount( fs );

	if ( strcmp( mount_path, "/" ) == 0 ){
		set_global_vfs_root( fs->root_node );

	} else {
		mount = knew( file_node_t );
		foo = file_lookup_absolute( mount_path, mount, 0 );
		kprintf( "[%s] lookup for \"%s\" returned %d, mount node at hash 0x%x\n", 
			__func__, mount_path, foo, vfs_node_hash( mount ));

		if ( foo >= 0 ){
			file_node_t *mountroot = knew( file_node_t );
			memcpy( mountroot, fs->root_node, sizeof( file_node_t ));
			file_register_mount( vfs_node_hash( mount ), mountroot );
			//ret = VFS_FUNCTION( mount, mount, fs->root_node, flags );
		}

		kfree( mount );
	}

done:
	return ret;

}

int file_lookup( char *path, file_node_t *buf, int flags ){
	int ret = -ERROR_INVALID_PATH;
	file_node_t *node;

	if ( path ){
		if ( path[0] == '/' ){
			node = get_local_vfs_root( );
			path++;

			if ( *path ){
				ret = file_lookup_relative( path, node, buf, flags );

			} else {
				*buf = *node;
				ret = 0;
			}

		} else {
			//node = get_local_vfs_root( );
			node = get_current_dir( );
			ret = file_lookup_relative( path, node, buf, flags );
		}
	}

	return ret;
}

int file_lookup_absolute( char *path, file_node_t *buf, int flags ){
	int ret = 0;
	file_node_t *root;
	
	//root = get_global_vfs_root( );
	root = get_local_vfs_root( );
	if ( !root || *path != '/' ){
		ret = -ERROR_NOT_FOUND;

	} else {
		ret = file_lookup_relative( path + 1, root, buf, flags );
	}

	return ret;
}

int file_lookup_relative( char *path, file_node_t *node, file_node_t *buf, int flags ){
	int ret = 0;
	int found = 0;
	int i;
	int lastbuf = 1;
	int vfs_function_ret;

	char *namebuf;
	char *pathptr = path;
	bool expecting_dir;

	file_node_t *move = node;
	file_node_t *nodebufs;
	file_node_t *current_buf;

	if ( path && buf ){
		namebuf = knew( char[MAX_FILENAME_SIZE] );
		current_buf = nodebufs = knew( file_node_t[2] );

		// Iterate through directories in path, searching for the next level down
		// until the given path is found or there's an error
		while ( !ret && !found && *pathptr ){

			// Copy first directory from path into buffer
			for ( i = 0; i < 256 && pathptr[i] && pathptr[i] != '/'; i++ )
				namebuf[i] = pathptr[i];

			namebuf[i] = 0;
			pathptr += i;
			expecting_dir = false;

			if ( *pathptr == '/' ){
				pathptr++;
				expecting_dir = true;
				expecting_dir = expecting_dir; // XXX: get compiler to shut up about unused variable
			}

			if ( strcmp( namebuf, "." ) == 0 )
				continue;

			kprintf( "[%s] Doing lookup at hash 0x%x...\n", __func__, vfs_node_hash( move ));

			vfs_function_ret = VFS_FUNCTION( move, lookup, current_buf, namebuf, flags );
			if ( vfs_function_ret ){
				kprintf( "[%s] Oops we got an error, %d\n", __func__, -vfs_function_ret );
				ret = vfs_function_ret;
				break;
			}

			kprintf( "[%s] Have current buffer at hash 0x%x\n", __func__, vfs_node_hash( current_buf ));

			if ( file_get_mount( current_buf, vfs_node_hash( current_buf ))){
				kprintf( "[%s] Found mount point, new hash is 0x%x\n", __func__, vfs_node_hash( current_buf ));

			} else {
				kprintf( "[%s] Didn't find any mountpoints for hash 0x%x\n", __func__, vfs_node_hash( current_buf ));
			}

			/*
			if ( current_buf->mount ){
				move = current_buf->mount->fs->root_node;
			} else {
				move = current_buf;
			}
			*/
			move = current_buf;

			/* This is probably obtuse enough to deserve a comment, so...
			 * There's two file node buffers which are alternated between during lookups.
			 * This is because of `move = current_buf`. With one buffer
			 * `move` will point to itself after one iteration, which means
			 * it'll be overwritten while still in use by the lookup function.
			 */
			current_buf = nodebufs + lastbuf;
			lastbuf = !lastbuf;
		}

		if ( !ret )
			memcpy( buf, move, sizeof( file_node_t ));

		kfree( nodebufs );
		kfree( namebuf );

	} else {
		ret = -ERROR_INVALID_PATH;
	}

	return ret;
}

int init_vfs( ){
	kprintf( "[%s] Initializing virtual file system...", provides );

	driver_list = list_add_data_node( driver_list, 0 );
	//mount_list = list_add_data_node( mount_list, 0 );
	mount_map = hashmap_create( 16 );

	p_driver_list = create_protected_var( 1, driver_list );
	//p_mount_list = create_protected_var( 1, mount_list );
	p_mount_map = create_protected_var( 1, mount_map );

	init_ramfs( );

	list_node_t *temp = driver_list;
	file_driver_t *drv;
	foreach_in_list( temp ){
		drv = temp->data;
		kprintf( "[%s] Have driver \"%s\"\n", provides, drv->name );
	}

	register_syscall( SYSCALL_OPEN,    vfs_open );
	register_syscall( SYSCALL_CLOSE,   vfs_close );
	register_syscall( SYSCALL_READ,    vfs_read );
	register_syscall( SYSCALL_WRITE,   vfs_write );
	register_syscall( SYSCALL_SPAWN,   vfs_spawn );
	register_syscall( SYSCALL_READDIR, vfs_readdir );
	register_syscall( SYSCALL_CHROOT,  vfs_chroot );
	register_syscall( SYSCALL_CHDIR,   vfs_chdir );
	register_syscall( SYSCALL_LSEEK,   vfs_lseek );
	register_syscall( SYSCALL_PIPE,    make_pipes );
	register_syscall( SYSCALL_FCNTL,   vfs_fcntl );
	register_syscall( SYSCALL_POLL,    vfs_poll );

	drv = file_get_driver( "ramfs" );
	kprintf( "[%s] Have ramfs at 0x%x\n", provides, drv );

	int stuff = file_mount_filesystem( "/", NULL, "ramfs", 0 );

	if ( stuff < 0 ){
		kprintf( "[%s] Could not set file root...\n", provides );

	} else {
		// set up the base vfs directories
		{
			file_node_t filebuf;

			file_lookup_absolute( "/", &filebuf, 0 );
			VFS_FUNCTION(( &filebuf ), mkdir, "helix", 0 );
			file_lookup_absolute( "/helix", &filebuf, 0 );
			VFS_FUNCTION(( &filebuf ), mkdir, "devices", 0 );
			VFS_FUNCTION(( &filebuf ), mkdir, "userroot", 0 );
		}
	}

	return 0;
}
