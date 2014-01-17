#ifndef _helix_vfs_module_h
#define _helix_vfs_module_h

enum {
	FILE_FLAG_NULL 		= 0,
	FILE_FLAG_MOUNTED 	= 1,
};

enum {
	FILE_DRIVER_NULL,
};

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
typedef struct dirp *(*opendir_func)( struct file_node * );
typedef int (*closedir_func)( struct file_node * );
typedef int (*close_func)( struct file_node * );

typedef int (*mkdir_func)( struct file_node *, char *, int );
typedef int (*mknod_func)( struct file_node *, char *, int, int );
typedef int (*link_func)( struct file_node *, struct file_node * );
typedef int (*unlink_func)( struct file_node * );
typedef int (*getdents_func)( struct file_node *, struct dirent *dirp, unsigned long count, unsigned long offset );
typedef int (*readdir_func)( struct file_node *, struct dirent *dirp, int entry );

// Functions provided by drivers to manage filesystems
typedef int (*file_create_fs)( struct file_driver *, struct file_system *, char *, unsigned flags );
typedef int (*file_remove_fs)( struct file_driver *, struct file_system *, unsigned flags );

typedef struct file_funcs {
	read_func  	read;
	pread_func  	pread;
	write_func	write;
	pwrite_func 	pwrite;

	mkdir_func	mkdir;
	mknod_func	mknod;
	link_func	link;
	unlink_func	unlink;
	readdir_func	readdir;

	open_func	open;
	close_func	close;

	file_get_info	get_info;
} file_funcs_t;

typedef struct file_system {
	void 	*devstruct; 	// Place for file driver to store it's precious memories

	file_funcs_t *functions;
} file_system_t;

typedef struct file_driver {
	void 	*driver;

	file_create_fs 	*create;
	file_remove_fs 	*remove;
} file_driver_t;

typedef struct file_node {
	unsigned 	inode;

	file_system_t 	*fs;
} file_node_t;

typedef struct file_info {
	file_type_t	type;

	//char		name[ MAX_NAME_LEN ];
	char		*name;
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
