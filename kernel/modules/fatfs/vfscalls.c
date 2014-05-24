#include <fatfs/fatfs.h>
#include <base/logger.h>
#include <base/kstd.h>
#include <base/string.h>

int fatfs_vfs_lookup( struct file_node *node, struct file_node *buf, char *name, int flags ){
	int ret = -ERROR_NOT_FOUND;
	fatfs_device_t *dev = node->fs->devstruct;

	unsigned cluster_size;
	char namebuf[256];
	uint8_t *sectbuf;
	int i;
	int has_longname = 0;

	fatfs_dirent_t *dirbuf;
	fatfs_dircache_t *dircache;
	fatfs_dircache_t *newcache;
	fatfs_longname_ent_t *longentbuf;

	dircache = hashmap_get( dev->inode_map, node->inode );

	if ( dircache ){
		if ( dircache->dir.attributes & FAT_ATTR_DIRECTORY ){

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
						newcache = knew( fatfs_dircache_t );
						memcpy( &newcache->dir, dirbuf + i, sizeof( fatfs_dirent_t ));
						newcache->references = 0;

						buf->inode = fatfs_relclus_to_sect( dev, newcache->dir.cluster_low );
						buf->fs = node->fs;
						buf->mount = 0;

						if ( hashmap_get( dev->inode_map, buf->inode ) == 0 )
							hashmap_add( dev->inode_map, buf->inode, newcache );

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

int fatfs_vfs_read( struct file_node *node, void *buf, unsigned long length, unsigned long offset ){
	unsigned cluster_size;
	unsigned i, k;
	unsigned cluster;
	uint8_t *outbuf = buf;
	uint8_t *clusbuf;
	int ret = 0;

	fatfs_device_t *dev;
	fatfs_dircache_t *cache;

	dev = node->fs->devstruct;
	cache = hashmap_get( dev->inode_map, node->inode );
	cluster_size = dev->bpb->bytes_per_sect * dev->bpb->sect_per_clus;

	if ( cache ){
		if ( cache->dir.attributes & FAT_ATTR_ARCHIVE ){

			i = offset / cluster_size;
			cluster = cache->dir.cluster_low;

			for ( ; i; i-- )
				cluster = fatfs_get_next_cluster( dev, cluster );

			if ( cluster < FAT_CLUSTER_BAD ){
				clusbuf = knew( uint8_t[ cluster_size ]);

				i = offset % cluster_size;
				VFS_FUNCTION(( &dev->device_node ), read, clusbuf, cluster_size,
						fatfs_relclus_to_sect( dev, cluster ) * dev->bpb->bytes_per_sect );

				for ( k = 0; k < length && k < cache->dir.size; k++, i++ ){
					outbuf[k] = clusbuf[ i % cluster_size ];

					if (( i + 1 ) % cluster_size == 0 ){
						cluster = fatfs_get_next_cluster( dev, cluster );

						if ( cluster >= FAT_CLUSTER_BAD )
							break;

						VFS_FUNCTION(( &dev->device_node ), read, clusbuf, cluster_size,
								fatfs_relclus_to_sect( dev, cluster ) * dev->bpb->bytes_per_sect );
					}
				}

				ret = k;

				kfree( clusbuf );
			}

		} else {
			ret = -ERROR_IS_DIRECTORY;
		}

	} else {
		ret = -ERROR_NOT_FOUND;
	}

	return ret;
}

int fatfs_vfs_open( struct file_node *node, char *path, int flags ){
	fatfs_dircache_t *cache;
	fatfs_device_t *dev;
	int ret = 0;

	dev = node->fs->devstruct;
	cache = hashmap_get( dev->inode_map, node->inode );

	if ( cache ){
		cache->references++;
		ret = node->inode;
	} else {
		ret = -ERROR_NOT_FOUND;
	}

	return ret;
}

int fatfs_vfs_close( struct file_node *node ){
	fatfs_dircache_t *cache;
	fatfs_device_t *dev;
	int ret = 0;

	dev = node->fs->devstruct;
	cache = hashmap_get( dev->inode_map, node->inode );

	if ( cache ){
		if ( cache->references )
			cache->references--;

		if ( cache->references == 0 )
			hashmap_remove( dev->inode_map, node->inode );

		ret = 0;

	} else {
		ret = -ERROR_NOT_FOUND;
	}

	return ret;
}

//int fatfs_vfs_get_info( struct file_node *node, struct file_info *buf ){

int fatfs_vfs_readdir( struct file_node *node, struct dirent *dirp, int entry ){
	int ret = 0;

	return ret;
}
