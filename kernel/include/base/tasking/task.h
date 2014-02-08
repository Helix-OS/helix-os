#ifndef _helix_tasking_h
#define _helix_tasking_h

#include <base/arch/i386/pitimer.h> /* TODO: remove this, implement generic timer */
#include <base/datastructs/list.h>
#include <base/logger.h>
#include <base/string.h>
#include <base/tasking/semaphore.h>

enum {
	TASK_STATE_RUNNING,
	TASK_STATE_SLEEPING,
	TASK_STATE_WAITING,
};

typedef struct task {
	unsigned long 		state;
	unsigned long 		eip,
				esp,
				ebp;

	unsigned long 		pid;
	unsigned long 		tid;
	unsigned long 		sleep;
	unsigned long 		stack;

	unsigned long 		*pagedir;
	semaphore_t 	 	*sem;
} task_t;

extern unsigned long get_instruct_ptr( );

void init_tasking( void );
int is_task_blocked( );
void block_tasks( );
void unblock_tasks( );
void usleep( unsigned long useconds );

task_t *add_task( task_t *new_task );
int create_thread( void (*start)( ));
task_t *init_task( task_t *buffer );
int remove_task_by_pid( int pid );

list_node_t *get_task_list( void );
list_node_t *get_current_task_node( void );
task_t *get_current_task( void );
list_node_t *get_task_node_by_pid( unsigned pid );
unsigned long get_current_pid( );

void set_current_task_node( list_node_t *node );

#endif
