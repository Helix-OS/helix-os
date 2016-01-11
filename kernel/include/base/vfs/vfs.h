#ifndef _helix_vfs_module_h
#define _helix_vfs_module_h

#ifdef __cplusplus
extern "C" {
#endif

#include <base/errors.h>
#include <base/stdint.h>
#include <base/tasking/pobj.h>

// Defines
#define MAX_FILENAME_SIZE 	256
#define FILE_POBJ 		0xf11e

// Macros
#define VFS_FUNCTION( node, func, ... )\
	(!((node)->fs->functions && (node)->fs->functions->func )? -ERROR_NO_FUNC :\
	(node)->fs->functions->func((node), __VA_ARGS__ ))

// Flags
// File system flags
// TODO: Rename these, currently might be confused with general flags
enum {
	FILE_FLAG_NULL 		= 0,
	FILE_FLAG_READ  	= 1,
	FILE_FLAG_WRITE  	= 2,
	FILE_FLAG_EXEC  	= 4,
	FILE_FLAG_MOUNTED 	= 8,
	FILE_FLAG_INCOMPLETE 	= 16,
};

// Flags passed when doing node lookups
enum {
	FILE_LOOKUP_NULL 	= 0,
	FILE_LOOKUP_NOLINKS 	= 1, // don't follow symlinks
	FILE_LOOKUP_NOMOUNT 	= 2  // don't follow mount points
};

// File driver flags
enum {
	FILE_DRIVER_NULL 		= 0,
	FILE_DRIVER_ALLOW_NULL_PATH 	= 1,
};

// File node flags
enum {
	FILE_NODE_NULL 		= 0,
	FILE_NODE_CACHED 	= 1,
}; 

// General file flags, passed to open, mknod, etc
enum { 
	FILE_NULL,
	FILE_READ,
	FILE_WRITE,
	FILE_CREATE,
	FILE_TRUNCATE,
};

typedef enum {
	FILE_TYPE_NULL,
	FILE_TYPE_REG,
	FILE_TYPE_DIR,
} file_type_t;

typedef enum {
	FILE_SEEK_SET,
	FILE_SEEK_CUR,
	FILE_SEEK_END,
} file_seek_t;

// Structure prototypes for the following function prototypes
struct file_node;
struct file_info;
struct file_driver;
struct file_system;
struct dirent;

// Functions to manipulate nodes in a driver
typedef int (*file_get_info)( struct file_node *node, struct file_info *buf );

typedef int (*open_func)( struct file_node *, char *, int );
typedef int (*write_func)( struct file_node *, void *, size_t, size_t );
typedef int (*read_func)( struct file_node *, void *, size_t, size_t );
typedef int (*close_func)( struct file_node *, int flags );

typedef int (*mkdir_func)( struct file_node *, char *, int );
typedef int (*mknod_func)( struct file_node *, char *, int, int );
typedef int (*link_func)( struct file_node *, struct file_node * );
typedef int (*unlink_func)( struct file_node * );
typedef int (*readdir_func)( struct file_node *, struct dirent *dirp, int entry );
typedef int (*mount_func)( struct file_node *, struct file_node *, int flags );
typedef int (*lookup_func)( struct file_node *, struct file_node *, char *name, int flags );

// Functions provided by drivers to manage filesystems
typedef struct file_system *(*file_create_fs)( struct file_driver *,
		struct file_system *, char *, unsigned flags );
typedef int (*file_remove_fs)( struct file_driver *, struct file_system *, unsigned flags );

// VFS structures
// File system functions
typedef struct file_funcs {
	read_func  	read;
	write_func 	write;

	mkdir_func	mkdir;
	mknod_func	mknod;
	mount_func 	mount;
	link_func	link;
	unlink_func	unlink;
	readdir_func	readdir;
	lookup_func 	lookup;

	open_func	open;
	close_func	close;

	file_get_info	get_info;
} file_funcs_t;

// A file system
typedef struct file_system {
	struct file_node 	*root_node; // Top node of file system
	file_funcs_t 		*functions;

	unsigned 		references;
	void 			*devstruct; // device-specific storage
} file_system_t;

// File driver to provide file systems
typedef struct file_driver {
	unsigned 	references;
	unsigned 	flags;

	void 		*driver; // Reserved for private use by the driver
	char 		*name; 

	file_create_fs 	create;
	file_remove_fs 	remove;

} file_driver_t;

// A mount point
typedef struct file_mount {
	unsigned 	flags;

	file_system_t  	*fs;
} file_mount_t;

// A file node generated by a lookup function
typedef struct file_node {
	unsigned 	inode;
	unsigned 	flags;
	unsigned 	references; 	// Number of references to this node

	file_system_t 	*fs; 		// Owning file system
	file_mount_t 	*mount; 	// Mount point
} file_node_t;

// Information about a file, can be returned by fstat(2)
typedef struct file_info {
	file_type_t type;

	unsigned mask;
	unsigned uid;
	unsigned gid;
	unsigned time;
	unsigned inode;
	unsigned size;
	unsigned links;
	unsigned flags;
	unsigned dev_id;
	unsigned blocks;
	unsigned blocksize;

	// TODO: add time fields

} file_info_t;

/* Similar to linux's old struct dirent, for simplicity */
typedef struct dirent {
	unsigned 	inode;
	unsigned 	offset; // Unused for now
	unsigned 	length;
	file_type_t 	type;

	char 		name[256];
} dirent_t;

typedef struct file_pobj {
	base_pobj_t base;
	file_node_t node;

	unsigned read_offset;
	unsigned write_offset;
	char *path;
} file_pobj_t;

int file_register_driver( file_driver_t *driver );
file_driver_t *file_get_driver( char *name );
//int file_register_mount( file_system_t *fs );
file_mount_t *file_register_mount( file_system_t *fs );

int file_mount_filesystem( char *mount_path, char *device, char *filesystem, int flags );

int file_lookup( char *path, file_node_t *buf, int flags );
int file_lookup_relative( char *path, file_node_t *node, file_node_t *buf, int flags );
int file_lookup_absolute( char *path, file_node_t *buf, int flags );

void set_global_vfs_root( file_node_t *root );
file_node_t *get_global_vfs_root( );
file_node_t *get_local_vfs_root( );
file_node_t *get_current_dir( );

// Syscalls exposed to userland
int vfs_open( char *path, int flags );
int vfs_close( int pnode );
int vfs_read( int pnode, void *buf, int length );
int vfs_write( int pnode, void *buf, int length );
int vfs_spawn( int pnode, char *args[], char *envp[], int *fds );
int vfs_readdir( int pnode, dirent_t *dirp, int entry );

int vfs_chroot( char *path );
int vfs_chdir( char *path );
int vfs_lseek( int fd, long offset, int whence );

int init_vfs( );

#ifdef __cplusplus
}
#endif

#endif
