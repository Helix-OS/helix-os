//#include <base/syscalls.h>
#include <base/stdint.h>
#include <base/sysinfo.h>
#include <base/tasking/task.h>
#include <arch/paging.h>

int sysinfo( unsigned type, unsigned key, void *buf ){
	int ret = 0;

	switch( type ){
		case SYSINFO_NONE:
			break;

		case SYSINFO_KERNEL:
			ret = get_kernel_info( key, buf );
			break;

		case SYSINFO_PROCESS:
			ret = get_process_info( key, buf );
			break;

		default:
			ret = -1;
			break;
	}

	return ret;
}

int get_kernel_info( unsigned key, kinfo_t *buf ){
	int ret = 1;

	buf->version    = 1;
	buf->page_size  = PAGE_SIZE;
	buf->free_pages = get_nfree_pages( );

	return ret;
}

int get_process_info( unsigned key, proc_info_t *buf ){
	int ret = 0;
	list_node_t *tasklist = get_task_list( );
	task_t *task;
	unsigned i;

	for ( i = 0; tasklist && i < key; tasklist = tasklist->next, i++ );
	if ( i == key && tasklist ){
		task = tasklist->data;
		buf->pid = task->pid;
		buf->state = task->state;
		ret = 1;
	}

	return ret;
}

