#include <base/tasking/pobj.h>
#include <base/kstd.h>
#include <base/stdint.h>
#include <base/mem/alloc.h>
#include <base/string.h>
#include <base/tasking/task.h>

void *pobj_copy( base_pobj_t *obj ){
	void *ret = NULL;

	if ( obj ){
		base_pobj_t *buf = knew( uint8_t[obj->size] );
		ret = buf;

		if ( obj->ctor ){
			obj->ctor( obj, buf );

		} else {
			memcpy( buf, obj, obj->size );
		}
	}

	return ret;
}

void pobj_free( base_pobj_t *obj ){
	if ( obj ){
		if ( obj->dtor ){
			obj->dtor( obj );

		} else {
			kfree( obj );
		}
	}
}

int pobj_get( unsigned pnode, void **obj ){
	void *temp;
	task_t *cur_task;
	int ret = 0;

	cur_task = get_current_task( );

	if ( pnode < dlist_allocated( cur_task->pobjects )){
		 temp = dlist_get( cur_task->pobjects, pnode );

		if ( temp ){
			*obj = temp;

		} else {
			ret = -ERROR_NOT_FOUND;
		}
	} else {
		ret = -ERROR_INVALID_ARGUMENT;
	}

	return ret;
}
