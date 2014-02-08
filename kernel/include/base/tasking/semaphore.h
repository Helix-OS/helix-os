#ifndef _helix_tasking_semaphore_h
#define _helix_tasking_semaphore_h

typedef int semaphore_t;

typedef struct protected_var {
	semaphore_t sem;
	void *var;

	int flags;
} protected_var_t;

void init_semaphore( semaphore_t *sem, int nallowed );
int enter_semaphore( semaphore_t *sem );
int leave_semaphore( semaphore_t *sem );

protected_var_t *create_protected_var( int nallowed, void *var );
void *access_protected_var( protected_var_t *var );
void leave_protected_var( protected_var_t *var );
void delete_protected_var( protected_var_t *var );

#endif
