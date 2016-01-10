#ifndef _helix_datastruct_shared_h
#define _helix_datastruct_shared_h

#define NO_DTOR      NULL
#define DEFAULT_DTOR kfree

typedef void (*shared_dtor_t)( void *ptr );

typedef struct shared {
	void *data;
	unsigned references;
	shared_dtor_t dtor;
	// semaphore here, maybe
} shared_t;

shared_t *shared_new( void *data, shared_dtor_t dtor );

void *shared_get( shared_t *ptr );
shared_t *shared_aquire( shared_t *ptr );
void  shared_release( shared_t *ptr );

#endif
