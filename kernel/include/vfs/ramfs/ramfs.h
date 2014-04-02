#ifndef _helix_vfs_ramfs_h
#define _helix_vfs_ramfs_h
#include <vfs/vfs.h>
#include <base/datastructs/dlist.h>
#include <base/datastructs/list.h>
#include <base/tasking/semaphore.h>

typedef struct ramfs_head {
	dlist_container_t 	*nodes;
	file_system_t 		*fs; 		// Owning file system

	unsigned long 		nnodes;
	unsigned long 		first_free;
	unsigned long 		root_inode;
	semaphore_t 		lock; 		// Unused at the moment
} ramfs_head_t;

typedef struct ramfs_node {
	char 		*name;
	void 	 	*data; 	// Usage depends on info.type, will either be file data
				// or a dlist of ramfs_dirents
	semaphore_t 	lock; 	// Unused at the moment

	file_info_t 	info;
	file_mount_t 	*mount; // only used if info.type == FILE_TYPE_DIR
} ramfs_node_t;

typedef struct ramfs_dirent {
	char  		*name;
	unsigned long 	inode;
} ramfs_dirent_t;

file_system_t *create_ramfs( struct file_driver *,
		struct file_system *, char *, unsigned flags );
int init_ramfs( );

#endif
