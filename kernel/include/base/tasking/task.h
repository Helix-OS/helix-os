#ifndef _helix_tasking_h
#define _helix_tasking_h

#include <base/datastructs/list.h>
#include <base/datastructs/dlist.h>
#include <base/logger.h>
#include <base/string.h>
#include <base/tasking/semaphore.h>
#include <base/arch/i386/pitimer.h> /* TODO: remove this, implement generic timer */
#include <base/arch/i386/paging.h> 

typedef unsigned long pid_t;

enum {
	TASK_STATE_RUNNING,
	TASK_STATE_SLEEPING,
	TASK_STATE_WAITING,
	TASK_STATE_STOPPED,
	TASK_STATE_ENDED,
};

typedef struct task {
	unsigned long 		state;
	unsigned long 		eip,
				esp,
				ebp;

	pid_t 	 		pid;
	pid_t 			group; /* Process group, possibly switch with 'pid' and rename
					  current uses of 'pid' to use 'tid' (thread id) */
	pid_t	 		parent;

	unsigned long 		sleep;
	unsigned long 		stack;

	page_dir_t 		*pagedir;
	semaphore_t 	 	*sem;

	int 			waiting;
	int 			end_status;

	dlist_container_t 	*pobjects;
} task_t;

extern unsigned long get_instruct_ptr( );

void init_tasking( void );
int is_task_blocked( );
void block_tasks( );
void unblock_tasks( );
void usleep( unsigned long useconds );

int create_thread( void (*start)( ));
int create_process( void (*start)( ), char *argv[], char *envp[] );
void exit_thread( );
void exit_process( int status );
task_t *add_task( task_t *new_task );
task_t *init_task( task_t *buffer );
int remove_task_by_pid( int pid );
pid_t waitpid( pid_t id, int *status, int options );

list_node_t *get_task_list( void );
list_node_t *get_current_task_node( void );
task_t *get_current_task( void );
list_node_t *get_task_node_by_pid( pid_t pid );
unsigned long get_current_pid( );

void set_current_task_node( list_node_t *node );

#endif
