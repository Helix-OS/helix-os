#include <base/tasking/task.h>

static list_node_t *task_list = 0;
static unsigned int pidcount = 0;
static int tasking_initialized = 0;

unsigned long get_instruct_ptr( ){
	unsigned long ret;
	asm volatile( "mov (%esp), %eax" );
	asm volatile( "mov %%eax, %0" : "=r"(ret));
	return ret;
}

void init_tasking( void ){
	task_t *root_task;
	kprintf( "Hello there, from 0x%x\n", get_instruct_ptr( ));

	root_task = kmalloc( sizeof( task_t ));
	memset( root_task, 0, sizeof( task_t ));

	task_list = list_add_data_node( task_list, root_task );
}

list_node_t *get_task_list( ){
	return task_list;
}

task_t *add_task( task_t *new_task ){
	list_add_data_node( get_task_list( ), new_task );

	return new_task;
}
