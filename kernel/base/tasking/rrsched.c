/* Round robin scheduler */
#include <base/tasking/task.h>
#include <base/tasking/rrsched.h>

/* Switches tasks when called */
void rrschedule_call( void ){
	if ( is_task_blocked( ))
		return;

	block_tasks( );
	unsigned long volatile 	esp = 0,
			      	eip = 0,
			      	ebp = 0,
			      	check = 0;

	list_node_t *current_node = get_current_task_node( ),
		    *next_node = 0;
	task_t *move = 0,
	       *current = get_current_task( );

	if ( !current_node || !current ){
		unblock_tasks( );
		return;
	}

	eip = get_instruct_ptr( );
	if ( check ){
		unblock_tasks( );
		return;
	}

	check = 1;

	asm volatile( "mov %%esp, %0" : "=r"( esp ));
	asm volatile( "mov %%ebp, %0" : "=r"( ebp ));

	current->eip = eip;
	current->esp = esp;
	current->ebp = ebp;

	next_node = current_node;
	do {
		next_node = next_node->next? next_node->next : get_task_list( );
		move = next_node->data;
	} while (( move->state == TASK_STATE_SLEEPING && move->sleep > get_tick( )) ||
			( move->state == TASK_STATE_WAITING && !*move->sem ));

	move->state = TASK_STATE_RUNNING;

	set_current_task_node( next_node );

	eip = move->eip;
	esp = move->esp;
	ebp = move->ebp;
	set_kernel_stack( move->stack );

	unblock_tasks( );
	asm volatile( "\
		cli;           \
		mov %0, %%esi; \
		mov %1, %%esp; \
		mov %2, %%ebp; \
		sti;\
		jmp *%%esi" :: "r"(eip), "r"(esp), "r"(ebp));
}

