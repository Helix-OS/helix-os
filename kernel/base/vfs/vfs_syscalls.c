#include <vfs/vfs.h>
#include <base/tasking/task.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>
#include <base/tasking/elfload.h>
#include <base/stdint.h>

// Not a syscall, is a helper function
int vfs_get_pobj( int pnode, file_pobj_t **obj ){
	file_pobj_t *nodeobj;
	task_t *cur_task;
	int ret = 0;

	cur_task = get_current_task( );

	if ( pnode < dlist_allocated( cur_task->pobjects )){
		nodeobj = dlist_get( cur_task->pobjects, pnode );

		if ( nodeobj ){
			if ( nodeobj->type == FILE_POBJ ){
				*obj = nodeobj;

			} else {
				ret = -ERROR_NOT_FILE;
			}

		} else {
			ret = -ERROR_NOT_FOUND; 
		}
	} else {
		ret = -ERROR_INVALID_ARGUMENT;
	}

	return ret;
}

int vfs_open( char *path, int flags ){
	file_pobj_t *newobj;

	int lookup;
	int ret;
	int i;
	task_t *cur_task;
	//file_node_t dirnode;
	char *dirpath;
	char *fpath;

	newobj = knew( file_pobj_t );
	newobj->type = FILE_POBJ;
	newobj->path = strdup( path );
	dirpath = strdup( path );

	for ( i = strlen( path ); i; i-- ){
		if ( path[i-1] == '/' ){
			dirpath[i-1] = 0;
			fpath = dirpath + i;

			kprintf( "[%s] Looking for directory \"%s\", file \"%s\"\n", __func__, dirpath, fpath );
			break;
		}
	}

	lookup = file_lookup_absolute( dirpath, &newobj->node, 0 );

	if ( lookup == 0 ){
		// TODO: Add permission checking
		lookup = VFS_FUNCTION( &newobj->node, open, fpath, flags );

		if ( lookup >= 0 ){
			file_lookup_absolute( path, &newobj->node, 0 );

			cur_task = get_current_task( );
			ret = dlist_add( cur_task->pobjects, newobj );
		} 
	}

	if ( lookup < 0 ){
		ret = lookup;
		kfree( newobj->path );
		kfree( newobj );
	}

	memset( dirpath, 0, strlen( dirpath ));
	kfree( dirpath );

	return ret;
}

int vfs_close( int pnode ){
	int ret;
	file_pobj_t *nodeobj;

	if (( ret = vfs_get_pobj( pnode, &nodeobj )) >= 0 )
		ret = VFS_FUNCTION( &nodeobj->node, close, 0 );

	return ret;
}

int vfs_read( int pnode, void *buf, int length ){
	file_pobj_t *nodeobj;
	int ret = 0;

	if ( length && ( ret = vfs_get_pobj( pnode, &nodeobj )) >= 0 ){
		ret = VFS_FUNCTION( &nodeobj->node, read, buf,
				length, nodeobj->read_offset );

		nodeobj->read_offset += ret;
	}

	return ret;
}

int vfs_write( int pnode, void *buf, int length ){
	file_pobj_t *nodeobj;
	int ret = 0;

	if ( length && ( ret = vfs_get_pobj( pnode, &nodeobj )) >= 0 ){
		ret = VFS_FUNCTION( &nodeobj->node, write, buf,
				length, nodeobj->write_offset );

		nodeobj->write_offset += ret;
	}

	return ret;
}

int vfs_spawn( int pnode, char *args[], char *envp[], int flags ){
	int ret = 0;
	int vfsfunc;
	file_pobj_t *nodeobj;
	Elf32_Ehdr *header;
	file_info_t *info;

	if (( ret = vfs_get_pobj( pnode, &nodeobj )) >= 0 ){
		info = knew( file_info_t );

		if (( ret = VFS_FUNCTION( &nodeobj->node, get_info, info )) >= 0 ){
			kprintf( "[%s] Loading process from file %d, size = %d\n", __func__, pnode, info->size );
			header = knew( uint8_t[ info->size ]);

			vfsfunc = vfs_read( pnode, header, info->size );
			if ( vfsfunc == info->size )
				elfload_from_mem( header );
			else
				ret = vfsfunc;

			kfree( header );
		}

		kfree( info );
	}

	return ret;
}
