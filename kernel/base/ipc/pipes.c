#include <base/ipc/pipes.h>
#include <base/datastructs/pipe.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>
#include <base/tasking/task.h>
#include <base/vfs/vfs.h>

static int pipe_obj_ctor( base_pobj_t *oldobj, base_pobj_t *newobj );
static int pipe_obj_dtor( base_pobj_t *obj );

static int pipe_vfs_read( file_node_t *node, void *buf, size_t length, size_t offset );
static int pipe_vfs_write( file_node_t *node, void *buf, size_t length, size_t offset );
static int pipe_vfs_close( file_node_t *node, int flags );
static file_event_t pipe_vfs_poll( file_node_t *node, file_event_t mask );

static file_funcs_t pipe_vfs_funcs = {
	.read  = pipe_vfs_read,
	.write = pipe_vfs_write,
	.close = pipe_vfs_close,
	.poll  = pipe_vfs_poll,
};

// This is kind of an edge case of the VFS code. A pipe should
// never have a lookup, mount/umount, etc operation done on it, so
// the file_system_t here only needs .functions set.
//
// It will need more things when named pipes are implemented, though
static file_system_t pipe_vfs_fs = {
	.functions = &pipe_vfs_funcs,
};

file_pobj_t *make_pipe_pobj( ){
	file_pobj_t *ret = knew( file_pobj_t );
	pipe_t *temp;

	ret->base.type = FILE_POBJ;
	ret->base.size = sizeof( file_pobj_t );
	ret->base.ctor = pipe_obj_ctor;
	ret->base.dtor = pipe_obj_dtor;
	ret->node.fs = &pipe_vfs_fs;
	ret->file_type = PIPE_POBJ;

	temp = pipe_create( 1, 256, 0 );
	init_semaphore( &temp->sem, 1 );
	ret->node.data = shared_new( temp, DEFAULT_DTOR );

	return ret;
}

int pipe_obj_ctor( base_pobj_t *oldobj, base_pobj_t *newobj ){
	file_pobj_t *oldpipe = (void *)oldobj;
	file_pobj_t *newpipe = (void *)newobj;
	pipe_t *temp;
	kprintf( "[%s] Got here\n", __func__ );

	if ( oldpipe && newpipe ){
		newpipe->base = oldpipe->base;
		newpipe->node = oldpipe->node;
		newpipe->node.data = shared_aquire( oldpipe->node.data );
		newpipe->file_type = oldpipe->file_type;

		temp = shared_get( newpipe->node.data );

		switch ( newpipe->file_type ){
			case PIPE_POBJ:
				temp->writers++;
				kprintf( "[%s] Copying write end, now have %d writers\n",
						 __func__, temp->writers );
				break;

			case PIPELINE_POBJ:
				temp->readers++;
				kprintf( "[%s] Copying read end, now have %d readers\n",
						 __func__, temp->readers );
				break;

			default:
				kprintf( "[%s] Unknown pipe object, 0x%x\n", __func__, newpipe->file_type );
				break;
		}
	}

	return 0;
}

int pipe_obj_dtor( base_pobj_t *obj ){
	file_pobj_t *pipeobj = (void *)obj;

	if ( pipeobj ){
		pipe_t *temp = shared_get( pipeobj->node.data );

		switch ( pipeobj->file_type ){
			case PIPE_POBJ:
				temp->writers--;
				kprintf( "[%s] Freeing write end, now have %d writers\n",
						 __func__, temp->writers );
				break;

			case PIPELINE_POBJ:
				temp->readers--;
				kprintf( "[%s] Freeing read end, now have %d readers\n",
						 __func__, temp->readers );
				break;

			default:
				kprintf( "[%s] Unknown pipe object, 0x%x\n", __func__, pipeobj->file_type );
				break;
		}

		shared_release( pipeobj->node.data );
		kfree( pipeobj );
	}

	return 0;
}

static int pipe_vfs_read( file_node_t *node, void *buf, size_t length, size_t offset ){
	pipe_t *temp = shared_get( node->data );
	int ret = 0;

	enter_semaphore( &temp->sem );
	//kprintf( "[%s] Reading pipe with %d writers...\n", __func__, temp->writers );
	ret = pipeline_read( temp->bufs[0], buf, length );
	leave_semaphore( &temp->sem );

	if ( length && ret == 0 && temp->writers >= 1){
		ret = -ERROR_TRY_AGAIN;
	}

	/*
	   if ( !ret && temp->writers >= 1 ){
	   rrschedule_call( );

	   } else {
	   break;
	   }
   */

	/*
	kprintf( "[%s] Read of length %d, returning with %d\n",
			 __func__, length, ret );
			 */

	return ret;
}

static int pipe_vfs_write( file_node_t *node, void *buf, size_t length, size_t offset ){
	pipe_t *temp = shared_get( node->data );
	int ret = 0;

	enter_semaphore( &temp->sem );
	ret = pipe_write( temp, buf, length );
	//kprintf( "[%s] Writing pipe...\n", __func__ );
	leave_semaphore( &temp->sem );

	if ( length && ret == 0 && temp->readers > 1 ){
		ret = -ERROR_TRY_AGAIN;
	}

	/*
	if ( !ret && temp->readers > 1 ){
		task_t *cur = get_current_task( );
		rrschedule_call( );

	} else {
		break;
	}
	*/

	//kprintf( "[%s] Write of length %d, returning with %d\n", __func__, length, ret );

	return ret;
}

static int pipe_vfs_close( file_node_t *node, int flags ){
	return 0;
}

static file_event_t pipe_vfs_poll( file_node_t *node, file_event_t mask ){
	file_event_t ret = 0;
	pipe_t *temp = shared_get( node->data );
	pipeline_t *line = temp->bufs[0];

	ret |= pipe_writeable( temp )   ? FILE_EVENT_WRITABLE : 0;
	ret |= pipeline_readable( line )? FILE_EVENT_READABLE : 0;
	ret &= mask;

	return ret;
}

int make_pipes( int *fds ){
	task_t *cur = get_current_task( );
	file_pobj_t *writeend;
	file_pobj_t *readend;
	pipe_t *temp;

	writeend = make_pipe_pobj( );
	readend = pobj_copy( &writeend->base );
	readend->file_type = PIPELINE_POBJ;

	temp = shared_get( writeend->node.data );
	temp->readers = 1;
	temp->writers = 1;

	fds[0] = dlist_add( cur->pobjects, readend );
	fds[1] = dlist_add( cur->pobjects, writeend );
	kprintf( "[%s] Got here, returning fds %d and %d\n", __func__, fds[0], fds[1] );

	return 0;
}

