#include <base/vfs/vfs.h>
#include <base/tasking/task.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>
#include <base/tasking/elfload.h>
#include <base/stdint.h>
#include <base/debug.h>
#include <base/ipc/pipes.h>
#include <base/datastructs/pipe.h>

// Not a syscall, is a helper function
int vfs_get_pobj( int pnode, file_pobj_t **obj ){
	file_pobj_t *nodeobj;
	task_t *cur_task;
	int ret = 0;

	cur_task = get_current_task( );

	if ( pnode < dlist_allocated( cur_task->pobjects )){
		nodeobj = dlist_get( cur_task->pobjects, pnode );

		if ( nodeobj ){
			if ( nodeobj->base.type == FILE_POBJ ){
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

// TODO: split this function into smaller pieces
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
		newobj->base.type = FILE_POBJ;
		newobj->base.size = sizeof( file_pobj_t );
		dirpath = strdup( path );

		for ( i = strlen( path ); i; i-- ){
			if ( path[i-1] == '/' ){
				dirpath[i-1] = 0;
				fpath = dirpath + i;
				dirfound = true;

				//kprintf( "[%s] Looking for directory \"%s\", file \"%s\"\n", __func__, dirpath, fpath );
				debugp( DEBUG_VFS, MASK_DEVINFO,
						"[%s] Looking for directory \"%s\", file \"%s\"\n", __func__, dirpath, fpath );
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

				// check to see if the path is not just "/"
				if ( *fpath ){
					lookup = VFS_FUNCTION( &newobj->node, open, fpath, flags );

					if ( lookup >= 0 ){
						file_lookup( path, &newobj->node, 0 );

						cur_task = get_current_task( );
						ret = dlist_add( cur_task->pobjects, newobj );
					}

				} else {
					// if it is "/", the root dir is already in &newobj->node
					// so just add it
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
	pipe_pobj_t *pipeobj;
	int ret = -ERROR_NOT_FILE;

	if ( length && ( ret = vfs_get_pobj( pnode, &nodeobj )) >= 0 ){
		ret = VFS_FUNCTION( &nodeobj->node, read, buf,
				length, nodeobj->read_offset );

		nodeobj->read_offset += ret;

	} else if ( pobj_get( pnode, (void **)&pipeobj ) >= 0 && pipeobj->base.type == PIPELINE_POBJ ){
		pipe_t *temp = shared_get( pipeobj->pipe );
		ret = pipeline_read( temp->bufs[0], buf, length );
	}


	return ret;
}

int vfs_write( int pnode, void *buf, int length ){
	file_pobj_t *nodeobj;
	pipe_pobj_t *pipeobj;
	int ret = 0;

	if ( length && ( ret = vfs_get_pobj( pnode, &nodeobj )) >= 0 ){
		ret = VFS_FUNCTION( &nodeobj->node, write, buf,
				length, nodeobj->write_offset );

		nodeobj->write_offset += ret;

	} else if ( pobj_get( pnode, (void **)&pipeobj ) >= 0 && pipeobj->base.type == PIPE_POBJ ){
		pipe_t *temp = shared_get( pipeobj->pipe );
		ret = pipe_write( temp, buf, length );
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
		kfree( temp );
	}

	return ret;
}

int vfs_chdir( char *path ){
	task_t *cur = get_current_task( );
	file_node_t *temp;
	int ret = 0;

	if ( cur->curdir ){
		temp = cur->curdir;

	} else {
		temp = knew( file_node_t );
	}

	ret = file_lookup( path, temp, 0 );
	debugp( DEBUG_VFS, MASK_CHECKPOINT, "[%s] Got here, 0x%x, returning %d\n", __func__, temp, ret );

	if ( ret >= 0 ){
		cur->curdir = temp;

	} else {
		kfree( temp );
	}

	return ret;
}

/*
int vfs_fstat( int fd, file_info_t *infobuf ){
	int ret = -1;
	return ret;
}
*/

int vfs_lseek( int fd, long offset, int whence ){
	int ret = -1;
	file_pobj_t *nodeobj;
	file_info_t *info;
	unsigned size;

	if (( ret = vfs_get_pobj( fd, &nodeobj) >= 0 )) {
		info = knew( file_info_t );

		if (( ret = VFS_FUNCTION( &nodeobj->node, get_info, info )) >= 0 ){
			//unsigned size = info->size;
			size = info->size;

			// TODO: possibly make these unsigned comparisons?
			switch ( whence ){
				case FILE_SEEK_SET:
					kprintf( "[%s] Seek set: %d\n", __func__, offset );
					if ( offset <= size ){
						nodeobj->read_offset = offset;
						nodeobj->write_offset = offset;
						ret = offset;

					} else {
						ret = -ERROR_INVALID_OFFSET;
					}
					break;

				case FILE_SEEK_CUR:
					kprintf( "[%s] Seek cur: %d\n", __func__, nodeobj->read_offset + offset );
					if ( nodeobj->read_offset + offset <= size ){
						nodeobj->read_offset += offset;
						nodeobj->write_offset = nodeobj->read_offset;
						ret = nodeobj->read_offset;

					} else {
						ret = -ERROR_INVALID_OFFSET;
					}

					break;

				case FILE_SEEK_END:
					kprintf( "[%s] Seek end: %d\n", __func__, size + offset );
					if ( size + offset <= size ){
						nodeobj->read_offset = size + offset;
						nodeobj->write_offset = nodeobj->read_offset;
						ret = nodeobj->read_offset;

					} else {
						ret = -ERROR_INVALID_OFFSET;
					}

					break;

				default:
					kprintf( "[%s] Invalid seek: %d, %d\n", __func__, fd, offset );
					ret = -ERROR_INVALID_ARGUMENT;
					break;
			}
		}

		kfree( info );
	}

	debugp( DEBUG_VFS, MASK_CHECKPOINT, "[%s] Got here, %u, %u, returning %d\n",
		__func__, size, nodeobj->read_offset + size, ret );

	return ret;
}
