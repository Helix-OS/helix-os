#include <fatfs/fatfs.h>
#include <base/logger.h>
#include <base/kstd.h>
#include <base/string.h>

char *depends[] = { "base", 0 };
char *provides = "fatfs";

static void dump_root_entries( fatfs_device_t *dev );

static file_funcs_t fatfs_functions = {
	.lookup = 0,
};

struct file_system *fatfs_create( struct file_driver *device, struct file_system *unused,
				char *path, unsigned flags )
{
	file_system_t *ret;
	fatfs_device_t *dev;
	file_node_t *root;
	unsigned first_data_sector;

	ret = knew( file_system_t );
	dev = ret->devstruct = knew( fatfs_device_t );

	if ( file_lookup_absolute( path, &dev->device_node, 0 ) >= 0 ){

		ret->root_node = root = knew( file_node_t );
		ret->functions = &fatfs_functions;
		dev->bpb = knew( uint8_t[512] );

		VFS_FUNCTION(( &dev->device_node ), read, dev->bpb, 512, 0 );

		dev->type = fatfs_get_type( dev->bpb );
		root->fs = ret;
		first_data_sector = dev->bpb->reserved_sects + dev->bpb->fats * dev->bpb->sects_per_fat;
		root->inode = first_data_sector;
		root->references = 1;

		kprintf( "[%s] FAT%d fs \"%s\" has %d bytes per sector, %d sectors per cluster,"
				"%d reserved sectors\n", __func__, dev->type,
				dev->bpb->oem_ident, dev->bpb->bytes_per_sect,
				dev->bpb->sect_per_clus, dev->bpb->reserved_sects );

		dump_root_entries( dev );

	} else {
		kfree( dev );
		kfree( ret );
		ret = 0;
	}

	return ret;
}

static void dump_root_entries( fatfs_device_t *dev ){
	unsigned first_data_sector;
	char namebuf[256];
	uint8_t *sectbuf;
	int i;
	int has_longname = 0;

	fatfs_dirent_t *dirbuf;
	fatfs_longname_ent_t *longentbuf;

	sectbuf = knew( uint8_t[512] ); 
	dirbuf = (void *)sectbuf;

	first_data_sector = dev->bpb->reserved_sects + dev->bpb->fats * dev->bpb->sects_per_fat;
	kprintf( "[%s] Reading directory from first data sector at 0x%x\n", __func__, first_data_sector );
	VFS_FUNCTION(( &dev->device_node ), read, sectbuf, 512, dev->bpb->bytes_per_sect * first_data_sector );

	kprintf( "[%s] Directories per cluster %d\n", __func__, dev->bpb->dirents );

	for ( i = 0; i < dev->bpb->dirents && i < 512 / sizeof( fatfs_dirent_t ); i++ ){

		if ( *(char *)(dirbuf + i) == 0 || *(char *)(dirbuf + i) == 0xe5 )
			continue;

		if ( dirbuf[i].attributes == FAT_ATTR_LONGNAME ){
			int j, index;
			has_longname = 1;

			longentbuf = (void *)(dirbuf + i);

			kprintf( "[%s] Have long name entry at %d, name index %d\n", __func__,
					i, longentbuf->name_index  );

			index = longentbuf->name_index - 'A';

			for ( j = 0; j < 5; j++ ) namebuf[ index * 13 + j      ] = (char)longentbuf->name_first[j];
			for ( j = 0; j < 6; j++ ) namebuf[ index * 13 + 5  + j ] = (char)longentbuf->name_second[j];
			for ( j = 0; j < 2; j++ ) namebuf[ index * 13 + 11 + j ] = (char)longentbuf->name_third[j];

		} else {
			char *fname = has_longname? namebuf : (char *)dirbuf[i].name;
			kprintf( "[%s] Have regular entry \"%s\" at %d with attributes %x, cluster at %d, size = %d bytes, at sector %d\n",
					__func__, fname, i, dirbuf[i].attributes, dirbuf[i].cluster_low, dirbuf[i].size,
					fatfs_relclus_to_sect( dev, dirbuf[i].cluster_low ));

			has_longname = 0;
			namebuf[0] = 0;
		}
	}

	kfree( sectbuf );
}

fat_type_t fatfs_get_type( fatfs_bpb_t *bpb ){
	fat_type_t ret = FAT_TYPE_NULL;
	unsigned data_sectors;
	unsigned rootdir_size; 
	unsigned total_clusters;

	rootdir_size = (bpb->dirents * 32 + bpb->bytes_per_sect - 1) / bpb->bytes_per_sect;
	data_sectors = bpb->sectors - (bpb->fats * bpb->sects_per_fat) - bpb->reserved_sects - rootdir_size;
	total_clusters = data_sectors / bpb->sect_per_clus;

	if ( total_clusters < 4085 )
		ret = FAT_TYPE_12;
	else if ( total_clusters < 65525 )
		ret = FAT_TYPE_16;
	else
		ret = FAT_TYPE_32;

	return ret;
}

unsigned fatfs_relclus_to_sect( fatfs_device_t *dev, unsigned cluster ){
	unsigned first_data_sector;
	unsigned root_cluster = 2;
	unsigned ret; // Return sector

	first_data_sector = dev->bpb->reserved_sects + dev->bpb->fats * dev->bpb->sects_per_fat;

	if ( cluster > root_cluster ){
		ret  = (cluster - root_cluster) * dev->bpb->sect_per_clus + first_data_sector;
		ret += dev->bpb->dirents * 32 / dev->bpb->bytes_per_sect; // Add sectors for the root directory

	} else { // cluster given /is/ the root directory
		ret = first_data_sector;
	}

	return ret;
}

int test( ){
	//file_node_t fnode;
	file_mount_filesystem( "/test/fatdir", "/test/devices/device1", "fatfs", 0 );

	return 0;
}

int init( ){
	file_driver_t *fatfs_driver;

	kprintf( "[%s] Hello world!\n", provides );
	kprintf( "[%s] This is module \"%s\", and I'm in yo kernelz\n",
			provides, provides );

	fatfs_driver = knew( file_driver_t );
	fatfs_driver->name = strdup( "fatfs" );
	fatfs_driver->create = fatfs_create;

	file_register_driver( fatfs_driver );
	test( );
	
	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
