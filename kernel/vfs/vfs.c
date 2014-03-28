#include <vfs/vfs.h>
#include <vfs/ramfs/ramfs.h>
#include <base/tasking/semaphore.h>
#include <base/datastructs/list.h>
#include <base/lib/stdbool.h>
#include <base/logger.h>
#include <base/string.h>

char *depends[] = { "base", 0 };
char *provides = "vfs";

static list_node_t *driver_list = 0;
static list_node_t *mount_list = 0;

static file_node_t *global_vfs_root;

static protected_var_t *p_driver_list;
static protected_var_t *p_mount_list;

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

int file_register_mount( file_system_t *fs ){
	list_node_t *mounts;
	file_mount_t *new_mount;

	mounts = access_protected_var( p_mount_list );
	new_mount = knew( file_mount_t );
	new_mount->fs = fs;
	new_mount->fs->references++;

	if ( !mounts->data ){
		mounts->data = new_mount;

	} else {
		list_add_data_node( mounts, new_mount );
	}

	leave_protected_var( p_mount_list );

	return 0;
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

void set_global_vfs_root( file_node_t *root ){
	global_vfs_root = root;
}

file_node_t *get_global_vfs_root( ){
	return global_vfs_root;
}

int file_mount_filesystem( char *mount_path, char *device, char *filesystem, int flags ){
	file_driver_t *driver;
	file_system_t *fs;
	file_node_t *mount = 0;
	int ret = 0;

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

	file_register_mount( fs );

	if ( strcmp( mount_path, "/" ) == 0 ){
		set_global_vfs_root( fs->root_node );

	} else {
		; //TODO Look up mount_path node

	}

done:
	return ret;

}

int file_lookup_absolute( char *path, file_node_t *buf, int flags ){
	int ret = 0;
	file_node_t *root;
	
	root = get_global_vfs_root( );
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
	char *namebuf,
	     *pathptr = path;
	bool expecting_dir;

	file_node_t *move = node;

	if ( path && buf ){
		namebuf = knew( char[MAX_FILENAME_SIZE] );

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
			}

			VFS_FUNCTION( move, lookup, buf, namebuf, flags );
			kprintf( "[%s] have directory name 0x%x:\"%s\", expecting directory: %d\n", __func__, *pathptr, namebuf, expecting_dir );
		}

		kfree( namebuf );

	} else {
		ret = -ERROR_INVALID_PATH;
	}

	return ret;
}

int init( ){
	kprintf( "[%s] Initializing virtual file system...", provides );

	driver_list = list_add_data_node( driver_list, 0 );
	mount_list = list_add_data_node( mount_list, 0 );

	p_driver_list = create_protected_var( 1, driver_list );
	p_mount_list = create_protected_var( 1, mount_list );

	init_ramfs( );

	list_node_t *temp = driver_list;
	file_driver_t *drv;
	foreach_in_list( temp ){
		drv = temp->data;
		kprintf( "[%s] Have driver \"%s\"\n", provides, drv->name );
	}

	drv = file_get_driver( "ramfs" );
	kprintf( "[%s] Have ramfs at 0x%x\n", provides, drv );

	int stuff = file_mount_filesystem( "/", NULL, "ramfs", 0 );
	file_node_t *blarg = get_global_vfs_root( );
	file_info_t *infos;

	if ( stuff < 0 || !blarg ){
		kprintf( "[%s] Could not get set file root...\n", provides );

	} else {
		infos = knew( file_info_t );

		kprintf( "[%s] blarg: 0x%x, fs: 0x%x, functions: 0x%x, get_info: 0x%x\n", 
				provides, blarg, blarg->fs,
				blarg->fs->functions, blarg->fs->functions->get_info );

		VFS_FUNCTION( blarg, get_info, infos );
		kprintf( "[%s] Have root node info: type = %d\n", provides, infos->type );
		file_lookup_absolute( "/usr/bin/pacman", blarg, 0 );
		file_lookup_relative( "pacman/asdf", blarg, blarg, 0 );

		kfree( infos );
	}

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 

}
