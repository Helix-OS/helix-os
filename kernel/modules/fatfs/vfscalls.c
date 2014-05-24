#include <fatfs/fatfs.h>
#include <base/logger.h>
#include <base/kstd.h>
#include <base/string.h>

int fatfs_vfs_lookup( struct file_node *node, struct file_node *buf, char *name, int flags ){
	int ret = -ERROR_NOT_FOUND;
	fatfs_device_t *dev = node->fs->devstruct;
	fatfs_dirent_t *dirent;

	unsigned cluster_size;
	char namebuf[256];
	uint8_t *sectbuf;
	int i;
	int has_longname = 0;

	fatfs_dirent_t *dirbuf;
	fatfs_dirent_t *clonedir;
	fatfs_longname_ent_t *longentbuf;

	dirent = hashmap_get( dev->inode_map, node->inode );

	if ( dirent ){
		if ( dirent->attributes & FAT_ATTR_DIRECTORY ){

			cluster_size = dev->bpb->bytes_per_sect * dev->bpb->sect_per_clus;
			sectbuf = knew( uint8_t[cluster_size] ); 
			dirbuf = (void *)sectbuf;

			VFS_FUNCTION(( &dev->device_node ), read, sectbuf,
					cluster_size, dev->bpb->bytes_per_sect * node->inode );

			for ( i = 0; i < dev->bpb->dirents; i++ ){

				if ( *(char *)(dirbuf + i) == 0 || *(char *)(dirbuf + i) == 0xe5 )
					continue;

				if ( dirbuf[i].attributes == FAT_ATTR_LONGNAME ){
					longentbuf = (void *)(dirbuf + i);
					fatfs_apply_longname( longentbuf, namebuf, 256 );
					has_longname = 1;

				} else {
					char *fname = has_longname? namebuf : (char *)dirbuf[i].name;

					if ( strcmp( name, fname ) == 0 ){
						// Found it, cool
						clonedir = knew( fatfs_dirent_t );
						memcpy( clonedir, dirbuf + i, sizeof( fatfs_dirent_t ));

						buf->inode = fatfs_relclus_to_sect( dev, clonedir->cluster_low );
						buf->fs = node->fs;
						buf->mount = 0;

						if ( hashmap_get( dev->inode_map, buf->inode ) == 0 )
							hashmap_add( dev->inode_map, buf->inode, clonedir );

						ret = 0;
						break;
					}

					has_longname = 0;
					namebuf[0] = 0;
				}
			}

			kfree( sectbuf );

		} else {
			ret = -ERROR_NOT_DIRECTORY;
		}

	} else {
		ret = -ERROR_NOT_FOUND;
	}
	
	return ret;
}

int fatfs_vfs_readdir( struct file_node *node, struct dirent *dirp, int entry ){
	int ret = 0;

	return ret;
}
