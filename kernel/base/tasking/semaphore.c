#include <base/tasking/semaphore.h>
#include <base/tasking/rrsched.h>
#include <base/tasking/task.h>
#include <base/mem/alloc.h>
#include <base/logger.h>

void init_semaphore( semaphore_t *sem, int nallowed ){
	if ( !sem )
		return;

	*sem = nallowed;
}

void semaphore_wait( semaphore_t *sem ){
	task_t *current;

	current = get_current_task( );
	current->state = TASK_STATE_WAITING;
	current->sem = sem;

	rrschedule_call( );
}

protected_var_t *create_protected_var( int nallowed, void *var ){
	protected_var_t *ret = 0;

	ret = kmalloc( sizeof( protected_var_t ));
	memset( ret, 0, sizeof( protected_var_t ));

	ret->sem = nallowed;
	ret->var = var;

	return ret;
}

void *access_protected_var( protected_var_t *var ){
	void *ret;

	enter_semaphore( &var->sem );
	ret = var->var;
	//kprintf( "Have ret = 0x%x\n", ret );

	return ret;
}

void leave_protected_var( protected_var_t *var ){
	leave_semaphore( &var->sem );
}

void delete_protected_var( protected_var_t *var ){

	kfree( var );
}

