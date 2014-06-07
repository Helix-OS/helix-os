#include <base/tasking/task.h>
#include <base/arch/i386/init_tables.h>
#include <base/tasking/rrsched.h>
#include <base/string.h>

static list_node_t *task_list = 0;
static list_node_t *current_task = 0;

static unsigned int pidcount = 0;
static int tasking_initialized = 0;
static int task_blocked = 0;

/*
unsigned long get_instruct_ptr( void ){
	asm volatile( "mov (%esp), %eax" );
	asm volatile( "ret" );
	return 0;
}
*/

void init_tasking( void ){
	asm volatile( "cli" );
	task_t *root_task;

	root_task = kmalloc( sizeof( task_t ));
	memset( root_task, 0, sizeof( task_t ));

	current_task = task_list = list_add_data_node( task_list, root_task );
	root_task->pagedir = get_current_page_dir( );
	root_task->pobjects = dlist_create( 0, 0 );
	tasking_initialized = 1;

	register_pitimer_call( rrschedule_call );
	asm volatile( "sti" );
}

int create_thread( void (*start)( )){
	task_t *new_task = 0;

	block_tasks( );

	new_task = kmalloc( sizeof( task_t ));
	memset( new_task, 0, sizeof( task_t ));

	new_task->pid = ++pidcount;
	new_task->group = get_current_task( )->group;
	new_task->state = TASK_STATE_RUNNING;

	new_task->pagedir = get_current_page_dir( );
	new_task->stack = (unsigned long)(kmalloc( 0x800 ));
	new_task->eip = (unsigned long)start;
	new_task->esp = (unsigned long)( new_task->stack + 0x800 );
	new_task->ebp = 0;
	add_task( new_task );

	new_task->pobjects = dlist_create( 0, 0 );

	unblock_tasks( );

	return new_task->pid;
}

int create_process( void (*start)( ), char *argv[], char *envp[] ){
	task_t *new_task;

	block_tasks( );

	new_task = kmalloc( sizeof( task_t ));
	memset( new_task, 0, sizeof( task_t ));

	new_task->pid = ++pidcount;
	new_task->group = new_task->pid;
	new_task->state = TASK_STATE_RUNNING;

	// Assume page directory is already set up by loader
	new_task->pagedir = get_current_page_dir( );
	new_task->pobjects = dlist_create( 0, 0 );

	new_task->stack = (unsigned long)(kmalloc( 0x800 ));
	//map_page( new_task->pagedir, 0xbfffe000 | PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT );
	//new_task->stack = 0xbfffe000;
	new_task->eip = (unsigned long)start;
	new_task->esp = (unsigned long)( new_task->stack + 0x800 );
	new_task->ebp = 0;

	// Copies all the argument and environment pointers to the task
	{
		int argc, i, envc,
		    slen;
		char **args;
		char **envs;

		for ( argc = 0; argv[argc]; argc++ );
		for ( envc = 0; envp[envc]; envc++ );

		new_task->esp -= sizeof( char *[argc + 1] );
		args = (char **)new_task->esp;

		new_task->esp -= sizeof( char *[envc + 1] );
		envs = (char **)new_task->esp;

		for ( i = 0; i < argc; i++ ){
			slen = strlen( argv[i] );
			new_task->esp -= sizeof( char[slen + 1] );

			args[i] = (char *)new_task->esp;
			strncpy( args[i], argv[i], slen );
		}
		args[argc] = 0;

		for ( i = 0; i < envc; i++ ){
			slen = strlen( envp[i] );
			new_task->esp -= sizeof( char[slen + 1] );

			envs[i] = (char *)new_task->esp;
			strncpy( envs[i], envp[i], slen );
		}
		envs[envc] = 0;

		new_task->esp -= sizeof( char ** );
		*((char ***)new_task->esp) = envs;

		new_task->esp -= sizeof( char ** );
		*((char ***)new_task->esp) = args;

		new_task->esp -= sizeof( int );
		*((int *)new_task->esp) = argc;

		kprintf( "[%s] Loading process with %d args at 0x%x\n", __func__, *((int *)new_task->esp), new_task->esp );
	}

	add_task( new_task );
	unblock_tasks( );

	return new_task->pid;
}

void exit_thread( ){
	int pid = get_current_pid( );

	if ( pid ){ // Don't remove task if it's the main kernel thread
		remove_task_by_pid( pid );
	}

	rrschedule_call( );
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

list_node_t *get_task_node_by_pid( unsigned pid ){
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

	block_tasks( );
	node = get_task_node_by_pid( pid );
	if ( !node )
		return -1;

	// Remove node from scheduler
	task = node->data;
	list_remove_node( node );

	// Clean up stack
	if ( task->stack ){
		kprintf( "[remove_task_by_pid] freeing thread stack at 0x%x\n", task->stack );
		kfree((void *)( task->stack ));
	}

	// Clean up process objects
	nobjs = dlist_allocated( task->pobjects );
	for ( i = 0; i < nobjs; i++ ){
		objptr = dlist_get( task->pobjects, i );
		if ( objptr )
			kfree( objptr );
	}

	kfree( task->pobjects );

	// And done
	kfree( task );
	unblock_tasks( );
	rrschedule_call( );

	return 0;
}

void set_current_task_node( list_node_t *node ){
	if ( node )
		current_task = node;

}

void usleep( unsigned long useconds ){
	block_tasks( );
	task_t *task = get_current_task( );

	task->sleep = get_tick( ) + (useconds * 100 / 1000);
	task->state = TASK_STATE_SLEEPING;

	unblock_tasks( );
	rrschedule_call( );
}

