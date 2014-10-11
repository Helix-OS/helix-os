#include <vfs/vfs.h>
#include <base/tasking/task.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>
#include <base/tasking/elfload.h>
#include <base/stdint.h>
#include <base/debug.h>

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
	int ret = -ERROR_NOT_FOUND;
	int i;
	task_t *cur_task;
	char *dirpath;
	char *fpath;
	bool dirfound = false;

	debugp( DEBUG_VFS, MASK_CHECKPOINT, "[%s] Got here, 0x%x, %c\n", __func__, path, *(path + 3));

	if ( strlen( path )){
		newobj = knew( file_pobj_t );
		newobj->type = FILE_POBJ;
		//newobj->path = strdup( path );
		dirpath = strdup( path );

		for ( i = strlen( path ); i; i-- ){
			if ( path[i-1] == '/' ){
				dirpath[i-1] = 0;
				fpath = dirpath + i;
				dirfound = true;

				kprintf( "[%s] Looking for directory \"%s\", file \"%s\"\n", __func__, dirpath, fpath );
				break;
			}
		}

		if ( dirfound ){
			if ( dirpath[0] )
				lookup = file_lookup( dirpath, &newobj->node, 0 );
			else 
				lookup = file_lookup_absolute( "/", &newobj->node, 0 );

			if ( lookup == 0 ){
				// TODO: Add permission checking
				lookup = VFS_FUNCTION( &newobj->node, open, fpath, flags );

				if ( lookup >= 0 ){
					//file_lookup_absolute( path, &newobj->node, 0 );
					file_lookup( path, &newobj->node, 0 );

					cur_task = get_current_task( );
					ret = dlist_add( cur_task->pobjects, newobj );
				} 
			}

			if ( lookup < 0 ){
				ret = lookup;
				kfree( newobj );
			}

		} else {
			lookup = VFS_FUNCTION( get_current_dir( ), open, path, flags );

			if ( lookup >= 0 ){
				//file_lookup_absolute( path, &newobj->node, 0 );
				file_lookup( path, &newobj->node, 0 );

				cur_task = get_current_task( );
				ret = dlist_add( cur_task->pobjects, newobj );
			} 

			if ( lookup < 0 ){
				ret = lookup;
				kfree( newobj );
			}
		}

		memset( dirpath, 0, strlen( dirpath ));
		kfree( dirpath );
	}

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

int vfs_readdir( int pnode, dirent_t *dirp, int entry ){
	file_pobj_t *nodeobj;
	int ret = 0;

	if (( ret = vfs_get_pobj( pnode, &nodeobj )) >= 0 ){
		ret = VFS_FUNCTION( &nodeobj->node, readdir, dirp, entry );
		kprintf( "[%s] Woot got here, ret = %d\n", __func__, -ret );
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
				ret = elfload_from_mem( header, args, envp );
			else
				ret = vfsfunc;

			kfree( header );
		}

		kfree( info );
	}

	return ret;
}

int vfs_chroot( char *path ){
	task_t *cur = get_current_task( );
	file_node_t *temp;
	int ret = 0;

	if ( cur->froot ){
		temp = cur->froot;

	} else {
		temp = knew( file_pobj_t );
	}

	ret = file_lookup( path, temp, 0 );
	debugp( DEBUG_VFS, MASK_CHECKPOINT, "[%s] Got here, 0x%x, returning %d\n", __func__, temp, ret );

	if ( ret >= 0 ){
		cur->froot = temp;
	} else {
		free( temp );
	}

	return ret;
}

int vfs_chdir( char *path ){
	task_t *cur = get_current_task( );
	file_node_t *temp;
	int ret = 0;

	if ( cur->froot ){
		temp = cur->curdir;

	} else {
		temp = knew( file_node_t );
	}

	ret = file_lookup( path, temp, 0 );

	if ( ret >= 0 ){
		cur->curdir = temp;

	} else {
		free( temp );
	}

	return ret;
}
