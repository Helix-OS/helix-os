//#include <base/syscalls.h>
#include <base/stdint.h>
#include <base/sysinfo.h>
#include <base/tasking/task.h>
#include <arch/paging.h>
#include <base/module.h>

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

		case SYSINFO_MODULES:
			ret = get_module_info( key, buf );
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
		buf->parent = task->parent;
		buf->p_group = task->group;
		buf->state = task->state;
		ret = 1;
	}

	return ret;
}

// TODO: add a getter function for this
extern list_head_t *mod_list;

int get_module_info( unsigned key, sysinfo_mod_info_t *buf ){
	int ret = 0;
	list_node_t *temp;

	temp = list_get_index( mod_list, key );
	kprintf( "[%s] Got here, %u, 0x%x\n", __func__, key, temp );

	if ( temp ){
		module_t *mod = temp->data;

		strncpy( buf->name, mod->name, 32 );
		buf->address = mod->address;
		buf->npages = mod->pages;
		ret = key + 1;
	}

	return ret;
}
