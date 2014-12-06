#ifndef _helix_tasking_semaphore_h
#define _helix_tasking_semaphore_h

typedef int semaphore_t;

typedef struct protected_var {
	semaphore_t sem;
	void *var;

	int flags;
} protected_var_t;

void init_semaphore( semaphore_t *sem, int nallowed );

protected_var_t *create_protected_var( int nallowed, void *var );
void *access_protected_var( protected_var_t *var );
void leave_protected_var( protected_var_t *var );
void delete_protected_var( protected_var_t *var );
void semaphore_wait( semaphore_t *sem );

static inline int enter_semaphore( semaphore_t *sem ){
	int ret;

	if ( *sem ){
		ret = --*sem;

	} else {
		semaphore_wait( sem );
		ret = --*sem;
	}

	return ret;
}

static inline int leave_semaphore( semaphore_t *sem ){
	return ++(*sem);
}

#endif
