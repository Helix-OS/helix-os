#ifndef _helix_tasking_h
#define _helix_tasking_h
#include <base/datastructs/list.h>
#include <base/logger.h>
#include <base/string.h>

typedef struct task {
	unsigned long 	eip,
			esp,
			ebp;

	unsigned long 	pid;
	unsigned long 	tid;
	unsigned long 	sleep;

	unsigned long 	*pagedir;
} task_t;

void init_tasking( void );
list_node_t *get_task_list( );

task_t *add_task( task_t *new_task );
task_t *init_task( task_t *buffer );

#endif
