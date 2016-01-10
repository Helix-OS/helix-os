#include <base/tasking/pobj.h>
#include <base/kstd.h>
#include <base/stdint.h>
#include <base/mem/alloc.h>
#include <base/string.h>

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
