#include <fatfs/fatfs.h>
#include <base/logger.h>
#include <base/kstd.h>
#include <base/string.h>
#include <base/lib/stdbool.h>

int fatfs_vfs_lookup( struct file_node *node, struct file_node *buf, char *name, int flags ){
	int ret = -ERROR_NOT_FOUND;
	fatfs_device_t *dev = node->fs->devstruct;

	unsigned cluster_size;
	//char namebuf[256];
	char *namebuf;
	uint8_t *sectbuf;
	int i;
	int has_longname = 0;

	fatfs_dirent_t *dirbuf;
	fatfs_dircache_t *dircache;
	fatfs_dircache_t *newcache;
	fatfs_longname_ent_t *longentbuf;

	dircache = hashmap_get( dev->inode_map, node->inode );

	if ( dircache ){
		namebuf = knew( char[ 256 ]);

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

		kfree( namebuf );
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

	kprintf( "[%s] Got here, %d\n", __func__, length );
	memset( buf, 'A', length );

	fatfs_device_t *dev;
	fatfs_dircache_t *cache;

	dev = node->fs->devstruct;
	cache = hashmap_get( dev->inode_map, node->inode );
	cluster_size = dev->bpb->bytes_per_sect * dev->bpb->sect_per_clus;

	if ( cache ){
		if ( cache->dir.attributes & FAT_ATTR_ARCHIVE ){

			i = offset / cluster_size;
			cluster = cache->dir.cluster_low;
			kprintf( "[%s] Have first cluster of file at 0x%x\n", __func__, cluster );

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
						kprintf( "[%s] cluster: 0x%x\n", __func__, cluster );

						if ( cluster >= FAT_CLUSTER_BAD )
							break;

						VFS_FUNCTION(( &dev->device_node ), read, clusbuf, cluster_size,
								fatfs_relclus_to_sect( dev, cluster ) * dev->bpb->bytes_per_sect );

						kprintf( "[%s] Read new cluster\n", __func__ );
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
	file_node_t nodebuf;
	int ret = 0;
	int lookup;

	dev = node->fs->devstruct;
	cache = hashmap_get( dev->inode_map, node->inode );

	if ( cache ){
		if ( cache->dir.attributes & FAT_ATTR_DIRECTORY ){
			lookup = file_lookup_relative( path, node, &nodebuf, 0 );

			if ( lookup == 0 ){
				/*
				cache->references++;
				ret = node->inode;
				*/
				cache = hashmap_get( dev->inode_map, nodebuf.inode );
				cache->references++;
				ret = nodebuf.inode;

			} else {
				ret = lookup;
			}

		} else {
			ret = -ERROR_NOT_DIRECTORY;
		}

	} else {
		ret = -ERROR_NOT_FOUND;
	}

	return ret;
}

int fatfs_vfs_close( struct file_node *node, int flags ){
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

int fatfs_vfs_get_info( struct file_node *node, struct file_info *buf ){
	int ret = 0;
	fatfs_dircache_t *cache;
	fatfs_device_t *dev;

	dev = node->fs->devstruct;
	cache = hashmap_get( dev->inode_map, node->inode );

	if ( cache ){
		if ( cache->dir.attributes & FAT_ATTR_DIRECTORY )
			buf->type = FILE_TYPE_DIR;
		else
			buf->type = FILE_TYPE_REG;

		buf->mask = 0555;
		buf->uid = buf->gid = 0;
		buf->inode = node->inode;
		buf->links = 1;
		buf->flags = 0;
		buf->dev_id = 0;
		buf->size = cache->dir.size;

	} else {
		ret = -ERROR_NOT_FOUND;
	}

	return ret;
}

int fatfs_vfs_readdir( struct file_node *node, struct dirent *dirp, int entry ){
	int ret = 0;
	fatfs_dircache_t *cache;
	fatfs_device_t *dev;
	fatfs_dirent_t *dirbuf;
	unsigned i;
	unsigned count;
	unsigned cluster_size;
	uint8_t *sectbuf;
	char *namebuf;
	bool found = false;

	dev = node->fs->devstruct;
	cache = hashmap_get( dev->inode_map, node->inode );
	cluster_size = dev->bpb->bytes_per_sect * dev->bpb->sect_per_clus;
	namebuf = knew( char[256] );
	sectbuf = knew( uint8_t[cluster_size] ); 

	dirbuf = (void *)sectbuf;

	VFS_FUNCTION(( &dev->device_node ), read, sectbuf,
			cluster_size, dev->bpb->bytes_per_sect * node->inode );

	/*
	kfree( namebuf );
	kfree( sectbuf );

	return 0;
	*/

	if ( cache ){
		if ( cache->dir.attributes & FAT_ATTR_DIRECTORY ){
			for ( count = i = 0; i < dev->bpb->dirents; i++ ){
				if ( *(char *)(dirbuf + i) == 0 ){
					break;

				} else if ( *(char *)(dirbuf + i) == 0xe5 ){
					continue;
				}

				if ( dirbuf[i].attributes == FAT_ATTR_LONGNAME ){
					fatfs_apply_longname((void *)(dirbuf + i), namebuf, 256 );

				} else {
					kprintf( "[%s] Got here, 0x%x, 0x%x\n", __func__, *(char *)(dirbuf + i), dirbuf[i].attributes );

					if ( count == entry ){
						found = true;
						break;
					}

					namebuf[0] = 0;
					count++;
				}
			}

			if ( found ){
				ret = 1;
				dirp->inode = fatfs_relclus_to_sect( dev, dirbuf[i].cluster_low );
				dirp->type = FILE_TYPE_REG;
				strncpy( dirp->name, namebuf, 256 );

			} else {
				ret = 0;
			}

		} else {
			ret = -ERROR_NOT_DIRECTORY;
		}

	} else {
		ret = -ERROR_NOT_FOUND;
	}

	kfree( namebuf );
	kfree( sectbuf );

	return ret;
}
