#ifndef _helix_vfs_module_h
#define _helix_vfs_module_h

enum {
	FILE_FLAG_NULL 		= 0,
	FILE_FLAG_READ  	= 1,
	FILE_FLAG_WRITE  	= 2,
	FILE_FLAG_EXEC  	= 4,
	FILE_FLAG_MOUNTED 	= 8,
	FILE_FLAG_INCOMPLETE 	= 16,
};

enum {
	FILE_LOOKUP_NULL 	= 0,
	FILE_LOOKUP_LINKS 	= 1, // follow symlinks
	FILE_LOOKUP_MOUNT 	= 2  // follow mount points
};

enum {
	FILE_DRIVER_NULL,
};

typedef enum {
	FILE_TYPE_NULL,
} file_type_t;

// Structure prototypes for the following function prototypes
struct file_node;
struct file_info;
struct file_driver;
struct file_system;
struct dirent;

// Functions to manipulate nodes in a driver
typedef int (*file_get_info)( struct file_node *node, struct file_info *buf );

typedef int (*open_func)( struct file_node *, char *, int);
typedef int (*write_func)( struct file_node *, void *, unsigned long );
typedef int (*read_func)( struct file_node *,  void *, unsigned long );
typedef int (*pwrite_func)( struct file_node *, void *, unsigned long, unsigned long );
typedef int (*pread_func)( struct file_node *, void *, unsigned long, unsigned long );
typedef int (*opendir_func)( struct file_node *, struct dirent *dirp );
typedef int (*closedir_func)( struct file_node * );
typedef int (*close_func)( struct file_node * );

typedef int (*mkdir_func)( struct file_node *, char *, int );
typedef int (*mknod_func)( struct file_node *, char *, int, int );
typedef int (*link_func)( struct file_node *, struct file_node * );
typedef int (*unlink_func)( struct file_node * );
typedef int (*getdents_func)( struct file_node *, struct dirent *dirp, unsigned long count, unsigned long offset );
typedef int (*readdir_func)( struct file_node *, struct dirent *dirp, int entry );
typedef int (*mount_func)( struct file_node *, struct file_node *, int flags );
typedef int (*lookup_func)( struct file_node *, struct file_node **, char *name, int flags );

// Functions provided by drivers to manage filesystems
typedef int (*file_create_fs)( struct file_driver *, struct file_system *, char *, unsigned flags );
typedef int (*file_remove_fs)( struct file_driver *, struct file_system *, unsigned flags );

// VFS structures
typedef struct file_funcs {
	// File system functions
	read_func  	read;
	pread_func  	pread;
	write_func	write;
	pwrite_func 	pwrite;

	mkdir_func	mkdir;
	mknod_func	mknod;
	link_func	link;
	unlink_func	unlink;
	readdir_func	readdir;
	lookup_func 	lookup;

	open_func	open;
	close_func	close;

	file_get_info	get_info;
} file_funcs_t;

typedef struct file_system {
	struct file_node *root_node; 	// Top node of file system
	file_funcs_t *functions;

	void 	*devstruct;  		// Place for file driver to store it's precious memories
} file_system_t;

typedef struct file_driver {
	unsigned 	references;
	unsigned 	flags;

	void 		*driver;
	char 		*name;

	file_create_fs 	*create;
	file_remove_fs 	*remove;
} file_driver_t;

typedef struct file_node {
	unsigned 	inode;
	unsigned 	flags;
	unsigned 	refcount; 	// Number of references to this node

	file_system_t 	*fs; 		// Owning file system
} file_node_t;

typedef struct file_mount {
	unsigned 	flags; 		// Uses same flags as file_system_t

	file_system_t  	*fs;
} file_mount_t;

typedef struct file_info {
	file_type_t	type;

	unsigned long 	mask;
	unsigned long	uid;
	unsigned long 	gid;
	unsigned long	time;
	unsigned long	inode;
	unsigned long	size;
	unsigned long	links;
	unsigned long 	flags;
	unsigned long	dev_id;

	unsigned long 	mount_id;
} file_info_t;

#endif
