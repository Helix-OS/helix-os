#include <vfs/vfs.h>
#include <base/datastructs/list.h>
#include <base/logger.h>

char *depends[] = { "base", 0 };
char *provides = "vfs";

static list_node_t *driver_list = 0;
static list_node_t *mount_list = 0;

int init( ){
	kprintf( "[%s] Initializing virtual file system...", provides );

	driver_list = list_add_data_node( driver_list, 0 );
	mount_list = list_add_data_node( driver_list, 0 );

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
