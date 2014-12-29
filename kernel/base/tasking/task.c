// TODO: reorganize this file so functions can be found easier
#include <base/tasking/task.h>
#include <base/tasking/rrsched.h>
#include <base/string.h>
#include <base/syscalls.h>
#include <base/debug.h>

//#include <arch/init_tables.h>

static list_node_t *task_list = 0;
static list_node_t *current_task = 0;

static unsigned int pidcount = 0;
static int tasking_initialized = 0;
static int task_blocked = 0;

static void idle_task( );

void init_tasking( void ){
	asm volatile( "cli" );
	task_t *root_task;

	root_task = kmalloc( sizeof( task_t ));
	memset( root_task, 0, sizeof( task_t ));

	current_task = task_list = list_add_data_node( task_list, root_task );
	root_task->pagedir = get_current_page_dir( );
	root_task->pobjects = dlist_create( 0, 0 );
	root_task->memmaps = 0;
	root_task->mainmap = 0;

	/*
	map_page( root_task->pagedir, 0xff000000 | PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER );
	flush_tlb( );

	root_task->stack = 0xff000000;
	*/
	root_task->stack = (unsigned long)kmalloca( 2048 );
	tasking_initialized = 1;

	//register_pitimer_call( rrschedule_call );
	register_timer_call( rrschedule_call );
	set_kernel_stack( root_task->stack );

	register_syscall( SYSCALL_EXIT, exit_process );
	register_syscall( SYSCALL_WAITPID, waitpid );
	register_syscall( SYSCALL_SBRK, sbrk );

	create_thread( idle_task );

	asm volatile( "sti" );
}

task_t *init_task( task_t *task ){
	task_t *cur = get_current_task( );
	task_t *ret = 0;

	task->pid = ++pidcount;
	task->group = cur->group;
	task->state = TASK_STATE_RUNNING;

	// If a process, assume the new directory has already been switched to
	task->pagedir = get_current_page_dir( );

	task->stack = 0xa1000000 + task->pid * 0x1000;
	map_page( task->pagedir, task->stack, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER );
	task->esp   = task->stack + 0x1000;
	flush_tlb( );
	/*
	task->stack = (unsigned long)(kmalloca( 0x800 ));
	task->esp = (unsigned long)( task->stack + 0x800 );
	*/
	task->ebp = 0;
	task->pobjects = dlist_create( 0, 0 );

	// TODO: Make file copying between processes work
	/*
	if ( cur->froot ){
		new_task->froot = knew( file_node_t );
		memcpy( new_task->froot, cur->froot, sizeof( file_node_t ));
	}

	if ( cur->curdir ){
		new_task->curdir = knew( file_node_t );
		memcpy( new_task->curdir, cur->curdir, sizeof( file_node_t ));
	}

	{
		int i;
		file_pobj_t *nfobj;
		file_pobj_t *nftemp;

		for ( i = 0; i < 3; i++ ){
			nfobj = dlist_get( cur->pobjects, i );

			if ( nfobj ){
				debugp( DEBUG_TASKING, MASK_CHECKPOINT, "[%s] Adding inode %d...\n", __func__, i );
				nftemp = knew( file_pobj_t );
				memcpy( nftemp, nfobj, sizeof( file_pobj_t ));

				dlist_add( new_task->pobjects, nftemp );

			} else {
				break;
			}
		}
	}
	*/
	
	return ret;
}

int create_thread( void (*start)( )){
	task_t *new_task = 0;
	task_t *cur = get_current_task( );

	block_tasks( );

	new_task = kmalloc( sizeof( task_t ));
	memset( new_task, 0, sizeof( task_t ));

	init_task( new_task );
	new_task->memmaps = cur->memmaps;
	new_task->mainmap = cur->mainmap;
	new_task->eip = (unsigned long)start;

	add_task( new_task );
	unblock_tasks( );

	return new_task->pid;
}

int create_process( void (*start)( ), char *argv[], char *envp[], list_head_t *map ){
	task_t *new_task;

	block_tasks( );

	new_task = kmalloc( sizeof( task_t ));
	memset( new_task, 0, sizeof( task_t ));

	init_task( new_task );
	new_task->eip = (unsigned long)start;

	if ( map ){
		list_node_t *temp = map->base;
		memmap_t *tempmap;

		new_task->memmaps = map;
		foreach_in_list( temp ){
			tempmap = temp->data;
			tempmap->references++;
		}

		new_task->mainmap = map->last->data;
	} else {
		// TODO: Add a dummy map here for paging purposes
		new_task->memmaps = list_create( 0 );
	}

	//map_page( new_task->pagedir, 0xa1000000 | PAGE_WRITEABLE | PAGE_USER | PAGE_PRESENT );
	//map_page( new_task->pagedir, 0xa1000000 | PAGE_WRITEABLE | PAGE_PRESENT );
	//flush_tlb( );

	/*
	new_task->stack = (unsigned long)(kmalloca( 0x800 ));
	new_task->esp = (unsigned long)( new_task->stack + 0x800 );
	//new_task->stack = 0xa1000000;
	//new_task->esp = (unsigned long)( new_task->stack + 0xf00 );
	*/
	new_task->eip = (unsigned long)start;
	new_task->ebp = new_task->esp;

	init_task_stack( new_task, argv, envp );

	add_task( new_task );
	unblock_tasks( );

	kprintf( "[%s] Done, returning\n", __func__ );

	return new_task->pid;
}

/** \brief Puts the given arguments and environment variables into place on the task's stack.
 * @param task The new task to add arguments to
 * @param args Null-terminated array of arguments
 * @param env  Null-terminated array of environment variables
 */
void init_task_stack( task_t *task, char *argv[], char *envp[] ){
	int argc, i, envc, slen;
	char **args;
	char **envs;

	// count number of arguments
	for ( argc = 0; argv[argc]; argc++ );
	// count number of environment variables
	for ( envc = 0; envp[envc]; envc++ );

	// allocate space on the task's stack for pointers to the arguments
	task->esp -= sizeof( char *[argc + 1] );
	args = (char **)task->esp;

	// and same for the environment variables
	task->esp -= sizeof( char *[envc + 1] );
	envs = (char **)task->esp;

	// copy each argument onto the task's stack
	for ( i = 0; i < argc; i++ ){
		slen = strlen( argv[i] );
		task->esp -= sizeof( char[slen + 1] );

		args[i] = (char *)task->esp;
		strncpy( args[i], argv[i], slen );
	}
	args[argc] = 0;

	// and again for the environment vars.
	for ( i = 0; i < envc; i++ ){
		slen = strlen( envp[i] );
		task->esp -= sizeof( char[slen + 1] );

		envs[i] = (char *)task->esp;
		strncpy( envs[i], envp[i], slen );
	}
	envs[envc] = 0;

	// push pointer to pointer to arguments onto the stack
	task->esp -= sizeof( char ** );
	*((char ***)task->esp) = envs;

	// push pointer to pointer to environment variables onto the stack
	task->esp -= sizeof( char ** );
	*((char ***)task->esp) = args;

	// and lastly push the number of arguments
	task->esp -= sizeof( int );
	*((int *)task->esp) = argc;

	/* final stack order is:
	 * |arg count      |
	 * |arg *[]        |
	 * |env *[]        |
	 * |env data       |
	 * |arg data       |
	 * |env *[argc]    |
	 * |arg *[envc]    |
	 */

	kprintf( "[%s] Loading process with %d args at 0x%x\n", __func__, *((int *)task->esp), task->esp );
}

void exit_process( int status ){
	task_t *cur = get_current_task( );

	if ( cur->pid ){
		cur->end_status = status;

		remove_task_by_pid( cur->pid );
	}

	rrschedule_call( );
}

void exit_thread( ){
	int pid = get_current_pid( );

	if ( pid ){ // Don't remove task if it's the main kernel thread
		remove_task_by_pid( pid );
	}

	rrschedule_call( );
}

pid_t waitpid( pid_t id, int *status, int options ){
	pid_t ret = id;
	task_t *target;
	list_node_t *listnode;

	listnode = get_task_node_by_pid( id );
	if ( !listnode )
		return -1;

	target = listnode->data;
	target->waiting++;

	kprintf( "[%s] Increased wait queue, %d\n", __func__, target->waiting );
	// TODO: properly find that the task has ended
	while ( target->state != TASK_STATE_ENDED ){
		//kprintf( "." );
		rrschedule_call( );
	}

	*status = target->end_status;
	target->waiting--;

	return ret;
}

void *sbrk( int increment ){
	task_t *cur;
	void *ret;
	
	cur = get_current_task( );
	ret = (void *)cur->mainmap->end;
	cur->mainmap->end += increment;

	kprintf( "[%s] Got here, returning 0x%x\n", __func__, ret );

	return ret;
}

unsigned long get_current_pid( ){
	unsigned long ret = 0;
	task_t *buf;

	buf = get_current_task( );
	ret = buf->pid;

	return ret;
}

list_node_t *get_task_list( void ){
	return task_list;
}

list_node_t *get_current_task_node( void ){
	return current_task;
}

task_t *get_current_task( void ){
	task_t *ret = 0;
	list_node_t *node;

	node = get_current_task_node( );
	if ( node ) ret = node->data;

	return ret;
}

list_node_t *get_task_node_by_pid( pid_t pid ){
	list_node_t *temp = get_task_list( );
	list_node_t *ret = 0;
	task_t *buf;

	foreach_in_list( temp ){
		buf = temp->data;
		if ( buf->pid == pid ){
			ret = temp;
			break;
		}
	}

	return ret;
}

int is_task_blocked( ){
	return task_blocked;
}

void block_tasks( ){
	task_blocked = 1;
}

void unblock_tasks( ){
	task_blocked = 0;
}

task_t *add_task( task_t *new_task ){
	if ( !tasking_initialized )
		return 0;

	list_add_data_node( get_task_list( ), new_task );

	return new_task;
}

int remove_task_by_pid( int pid ){
	list_node_t *node = 0;
	task_t *task;
	int i, nobjs;
	void *objptr;

	node = get_task_node_by_pid( pid );
	if ( !node )
		return -1;

	task = node->data;
	task->state = TASK_STATE_ENDED;
	kprintf( "[%s] waiting: %d\n", __func__, task->waiting );
	if ( task->waiting ){
		kprintf( "[%s] Letting other process get the exit status...\n", __func__ );

		rrschedule_call( );
	}

	block_tasks( );

	// Remove node from scheduler
	list_remove_node( node );

	// Clean up stack
	if ( task->stack ){
		kprintf( "[remove_task_by_pid] freeing thread stack at 0x%x\n", task->stack );
		//TODO: unmap stack pages here
		//kfree((void *)( task->stack ));
	}

	// FIXME: lots of use-after-free errors resulting from the commented code
	// Clean up process objects
	/*
	nobjs = dlist_allocated( task->pobjects );
	for ( i = 0; i < nobjs; i++ ){
		objptr = dlist_get( task->pobjects, i );
		if ( objptr )
			kfree( objptr );
	}

	kfree( task->pobjects );

	// And done
	kfree( task );
	*/
	unblock_tasks( );
	rrschedule_call( );

	return 0;
}

void set_current_task_node( list_node_t *node ){
	if ( node )
		current_task = node;

}

void usleep( unsigned long useconds ){
	//block_tasks( );
	task_t *task = get_current_task( );

	task->sleep = get_tick( ) + (useconds * 100 / 1000);
	task->state = TASK_STATE_SLEEPING;

	//unblock_tasks( );
	rrschedule_call( );
}

/** \brief Idle task which just calls the scheduler to switch tasks.
 *
 *  This is needed because the scheduler disables interrupts while looking for an
 *  task to run. If there's only one thread, and it does a usleep( ) call, the scheduler
 *  will go in to an infinite loop because get_tick( ) will never increase (because interrupts
 *  are disabled).
 *
 *  This gives anything that needs interrupts time to happen,
 *  without disabling interrupts in the scheduler.
 */
static void idle_task( ){
	while ( 1 )
		rrschedule_call( );
}
