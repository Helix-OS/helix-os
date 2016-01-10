#include <base/logger.h>
#include <base/datastructs/shared.h>
#include <base/mem/alloc.h>
#include <base/kstd.h>

shared_t *shared_new( void *data, shared_dtor_t dtor ){
	shared_t *ret = knew( shared_t );

	ret->data = data;
	ret->dtor = dtor;
	ret->references = 1;

	return ret;
}

void *shared_get( shared_t *ptr ){
	if ( ptr ){
		return ptr->data;
	}

	return NULL;
}

shared_t *shared_aquire( shared_t *ptr ){
	if ( ptr ){
		ptr->references++;
		/*
		kprintf( "[%s] Shared variable with data 0x%x now has %d references\n",
				__func__, ptr->data, ptr->references );
				*/
	}

	return ptr;
}

void shared_release( shared_t *ptr ){
	if ( ptr && ptr->references ){
		ptr->references--;

		/*
		kprintf( "[%s] Shared variable with data 0x%x now has %d references\n",
				__func__, ptr->data, ptr->references );
				*/

		if ( ptr->references == 0 ){
			if ( ptr->dtor ){
				ptr->dtor( ptr->data );
			}

			kfree( ptr );
		}

	} else {
		kprintf( "[%s] /!\\ Shared variable with data 0x%x has too many releases!\n",
				__func__, ptr->data );
	}
}
