// Defines structures and functions for dealing with per-process objects
#ifndef _helix_tasking_pobj_h
#define _helix_tasking_pobj_h

struct base_pobj;

typedef int (*pobj_ctor_t)( struct base_pobj *obj, struct base_pobj *newobj );
typedef int (*pobj_dtor_t)( struct base_pobj *obj );

typedef struct base_pobj {
	// TODO: add mutex or spinlock structure in here
	unsigned type;
	unsigned size;
	pobj_ctor_t ctor;
	pobj_dtor_t dtor;
} base_pobj_t;

void *pobj_copy( base_pobj_t *obj );
void  pobj_free( base_pobj_t *obj );
//void *pobj_get( unsigned node );
int pobj_get( unsigned pnode, void **obj );

#endif
