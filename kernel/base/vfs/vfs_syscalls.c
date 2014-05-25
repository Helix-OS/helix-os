#include <vfs/vfs.h>
#include <base/tasking/task.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>

int vfs_open( char *path, int flags ){
	file_pobj_t *newobj;

	int lookup;
	int ret;
	task_t *cur_task;

	newobj = knew( file_pobj_t );
	newobj->type = FILE_POBJ;
	lookup = file_lookup_absolute( path, &newobj->node, 0 );

	if ( lookup == 0 ){
		// TODO: Add permission checking
		lookup = VFS_FUNCTION( &newobj->node, open, path, flags );

		if ( lookup >= 0 ){
			cur_task = get_current_task( );
			ret = dlist_add( cur_task->pobjects, newobj );

		} else {
			ret = lookup;
			kfree( newobj );
		}

	} else {
		ret = lookup;
		kfree( newobj );
	}

	return ret;
}

int vfs_close( int pnode ){
	//file_node_t *node;
	file_pobj_t *nodeobj;
	task_t *cur_task;
	int ret;

	cur_task = get_current_task( );
	nodeobj = dlist_get( cur_task->pobjects, pnode );

	if ( nodeobj ){
		if ( nodeobj->type == FILE_POBJ )
			ret = VFS_FUNCTION( &nodeobj->node, close, 0 );
		else 
			ret = -ERROR_NOT_FILE;

	} else {
		ret = -ERROR_NOT_FOUND; 
	}

	return ret;
}

int vfs_read( int pnode, void *buf, int length ){
	int ret = 0;

	return ret;
}

int vfs_write( int pnode, void *buf, int length ){
	int ret = 0;

	return ret;
}

int vfs_spawn( int pnode, char *args[], char *envp[], int flags ){
	int ret = 0;

	return ret;
}

