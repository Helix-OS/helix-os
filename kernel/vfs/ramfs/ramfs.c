#include <vfs/ramfs/ramfs.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>
#include <base/string.h>
#include <base/lib/stdbool.h>

static int ramfs_get_node( ramfs_head_t *head, file_node_t *buf, int inode );
static int ramfs_node_get_info( struct file_node *node, struct file_info *buf );
static int ramfs_node_lookup( struct file_node *node, struct file_node *buf, char *name, int flags );
static ramfs_node_t *ramfs_add_node( ramfs_head_t *head, ramfs_node_t *dir, char *name );

static file_funcs_t ramfs_functions = {
	.get_info = ramfs_node_get_info,
	.lookup   = ramfs_node_lookup,
};

file_system_t *create_ramfs( struct file_driver *driver,
		struct file_system *unused, char *path, unsigned flags )
{
	ramfs_head_t *new_ramfs;
	ramfs_node_t *root_node,
		     *test_node;
	file_system_t *ret;
	file_node_t *fs_root;
	ramfs_dirent_t *dirbuf;

	new_ramfs = knew( ramfs_head_t );
	new_ramfs->nodes = dlist_create( 0, 0 );
	new_ramfs->nnodes = 0;
	new_ramfs->root_inode = 0;
	init_semaphore( &new_ramfs->lock, -1 );

	root_node = knew( ramfs_node_t );
	root_node->name = strdup( "root" );
	init_semaphore( &root_node->lock, -1 );

	// Set default file information
	root_node->info.type 	= FILE_TYPE_DIR;
	root_node->info.mask 	= 0777;
	root_node->info.inode 	= new_ramfs->nnodes++;
	root_node->info.links 	= 1;

	dlist_set( new_ramfs->nodes, 0, root_node );

	// Initialize the root node directory entries
	root_node->data = dlist_create( 0, 0 );

	test_node = ramfs_add_node( new_ramfs, root_node, "test" );
	test_node->info.type = FILE_TYPE_DIR;
	test_node->data = dlist_create( 0, 0 );

	test_node = ramfs_add_node( new_ramfs, test_node, "asdf" );

	ret = knew( file_system_t );
	ret->devstruct = new_ramfs;
	ret->functions = &ramfs_functions;

	new_ramfs->fs = ret;

	fs_root = ret->root_node = knew( file_node_t );
	ramfs_get_node( new_ramfs, fs_root, new_ramfs->root_inode );
	fs_root->references++;
	fs_root->fs = ret;

	return ret;
}

/** \brief Constructs a file_node_t from the given inode.
 *  @param head The filesystem structure, node->fs->devstruct from a file_node_t.
 *  @param buf  The output buffer
 *  @param inode The inode of the file to get a node from.
 */
static int ramfs_get_node( ramfs_head_t *head, file_node_t *buf, int inode ){
	ramfs_node_t *temp;
	int ret = -ERROR_NOT_FOUND;


	temp = dlist_get( head->nodes, inode );
	if ( temp ){
		kprintf( "[ramfs_get_node] Got here, head: 0x%x, head->fs: 0x%x\n", head, head->fs );
		buf->inode = temp->info.inode;
		buf->flags = 0;
		buf->references = 0;
		buf->mount = 0;
		buf->fs = head->fs;

		ret = 0;
	}

	return ret;
}

/** \brief Gets the internal node structure for an inode 
 *  @param head The filesystem structure
 *  @param buf  Pointer to the output pointer
 *  @param inode The inode of the file to get the structure for.
 */
static int ramfs_get_internal_node( ramfs_head_t *head, ramfs_node_t **buf, int inode ){
	ramfs_node_t *temp;
	int ret = -ERROR_NOT_FOUND;

	temp = dlist_get( head->nodes, inode );

	if ( temp ){
		*buf = temp;
		ret = 0;
	}
	
	return ret;
}

/** \brief Gets the file info struct for a given node
 *  @param node The node to get an info structure for
 *  @param buf  The output buffer
 */
static int ramfs_node_get_info( struct file_node *node, struct file_info *buf ){
	ramfs_head_t *head;
	ramfs_node_t *temp;
	int ret = -1;

	head = node->fs->devstruct;
	temp = dlist_get( head->nodes, node->inode );

	if ( temp ){
		memcpy( buf, &temp->info, sizeof( file_info_t ));
		ret = 0;
	}

	return ret;
}

/** \brief Looks up an entry with the given name in the directory, and returns a file node for it
 *  @param node The node to search
 *  @param buf  The output buffer
 *  @param name The name of the file to find
 *  @param flags Searching flags
 */
static int ramfs_node_lookup( struct file_node *node, struct file_node *buf, char *name, int flags ){

	ramfs_node_t *rnodebuf;
	ramfs_dirent_t *dirbuf;
	file_info_t infobuf;
	dlist_container_t *dlist;
	int listptr;
	int ret = -1;
	bool found = false;

	ramfs_node_get_info( node, &infobuf );

	if ( infobuf.type == FILE_TYPE_DIR ){
		kprintf( "[%s] Looking up %s... ", __func__, name );
		ramfs_get_internal_node( node->fs->devstruct, &rnodebuf, node->inode );
		dlist = rnodebuf->data;
		kprintf( "[%s] Have data pointer at 0x%x in struct 0x%x, node->fs: 0x%x\n",
				__func__, dlist, rnodebuf, node->fs );

		ret = -ERROR_NOT_FOUND;

		for ( listptr = 0; listptr < dlist->alloced && !found; listptr++ ){
			if ( dlist->entries[listptr] ){
				dirbuf = dlist->entries[listptr];
				if ( strcmp( name, dirbuf->name ) == 0 ){
					kprintf( "[%s] Found file %s at inode %d, cool\n",
							__func__, name, dirbuf->inode );
					ramfs_get_node( node->fs->devstruct, buf, dirbuf->inode );

					found = true;
					ret = 0;
				}
			}
		}

		if ( ret == -ERROR_NOT_FOUND )
			kprintf( "[%s] Couldn't find %s...\n", __func__, name );

	} else {
		ret = -ERROR_NOT_DIRECTORY;
		kprintf( "[%s] lolwut\n", __func__ );

	}

	return ret;
}

/** \brief Creates a ramfs node, adds it to the given directory, and returns it.
 *
 *  @param head The filesystem structure
 *  @param dir  The directory to add the node to
 *  @param name The name of the new node
 *
 *  @return A newly allocated ramfs node
 */
static ramfs_node_t *ramfs_add_node( ramfs_head_t *head, ramfs_node_t *dir, char *name ){
	ramfs_node_t *ret;
	ramfs_dirent_t *dirbuf;
	dlist_container_t *dlist = dir->data;
	int inode;
	
	ret = knew( ramfs_node_t );
	dirbuf = knew( ramfs_dirent_t );
	ret->data = 0;

	ret->name = strdup( name );
	dirbuf->name = strdup( name );

	ret->info.inode = dirbuf->inode = inode = dlist_add( head->nodes, ret );
	head->nnodes++;

	dlist_add( dlist, dirbuf );

	return ret;
}

int init_ramfs( ){
	file_driver_t *new_driver;

	new_driver = knew( file_driver_t );
	new_driver->name = strdup( "ramfs" ); 
	new_driver->create = create_ramfs;
	new_driver->flags = FILE_DRIVER_ALLOW_NULL_PATH;

	file_register_driver( new_driver );

	return 0;
}

void remove_ramfs( ){

}
