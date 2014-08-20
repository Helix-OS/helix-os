#include <fatfs/fatfs.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>

unsigned fatfs_relclus_to_sect( fatfs_device_t *dev, unsigned cluster ){
	unsigned first_data_sector;
	unsigned root_cluster = 2;
	unsigned ret; // Return sector

	kprintf( "[%s] got here\n", __func__ );

	first_data_sector = dev->bpb->reserved_sects + dev->bpb->fats * dev->bpb->sects_per_fat;

	if ( cluster > root_cluster ){
		ret  = (cluster - root_cluster) * dev->bpb->sect_per_clus + first_data_sector;
		// Add sectors for the root directory
		ret += dev->bpb->dirents * 32 / dev->bpb->bytes_per_sect;

	} else { // cluster given /is/ the root directory
		ret = first_data_sector;
	}

	return ret;
}

unsigned fatfs_get_next_cluster( fatfs_device_t *dev, unsigned cluster ){
	unsigned ret = FAT_CLUSTER_END;

	kprintf( "[%s] got here, cluster: 0x%x\n", __func__, cluster );

	switch( dev->type ){
		case FAT_TYPE_12:
			ret = fatfs_get_next_cluster_fat12( dev, cluster );
			break;

		case FAT_TYPE_16:
			ret = fatfs_get_next_cluster_fat16( dev, cluster );
			break;

		case FAT_TYPE_32:
			ret = fatfs_get_next_cluster_fat32( dev, cluster );
			break;

		default:
			break;
	}

	return ret;
}

unsigned fatfs_get_next_cluster_fat12( fatfs_device_t *dev, unsigned cluster ){
	unsigned ret = 0;
	unsigned fat_offset;
	unsigned fat_sector;
	unsigned ent_offset;

	unsigned cluster_size;
	uint16_t table_value;
	uint8_t  *fat_table;

	cluster_size = dev->bpb->bytes_per_sect * dev->bpb->sect_per_clus;
	fat_offset = cluster + (cluster / 2);
	fat_sector = dev->bpb->reserved_sects + (fat_offset / cluster_size);
	ent_offset = fat_offset % cluster_size;


	fat_table = knew( uint8_t[ cluster_size ]);
	VFS_FUNCTION(( &dev->device_node ), read, fat_table, cluster_size, fat_sector * dev->bpb->bytes_per_sect );

	table_value = *(uint16_t *)(fat_table + ent_offset);

	if ( cluster & 1 )
		ret = table_value >> 4;
	else
		ret = table_value & 0xfff;

	kfree( fat_table );

	kprintf( "[%s] Have next cluster at 0x%x\n", __func__, ret );
	return ret;
}

// TODO:
unsigned fatfs_get_next_cluster_fat16( fatfs_device_t *dev, unsigned cluster ){
	unsigned ret = 0;
	unsigned fat_offset;
	unsigned fat_sector;
	unsigned ent_offset;

	unsigned cluster_size;
	uint16_t table_value;
	uint8_t  *fat_table;

	cluster_size = dev->bpb->bytes_per_sect * dev->bpb->sect_per_clus;
	fat_offset = cluster * 2;
	fat_sector = dev->bpb->reserved_sects + (fat_offset / cluster_size);
	ent_offset = fat_offset % cluster_size;

	fat_table = knew( uint8_t[ cluster_size ]);
	VFS_FUNCTION(( &dev->device_node ), read, fat_table, cluster_size, fat_sector * dev->bpb->bytes_per_sect );

	table_value = *(uint16_t *)(fat_table + ent_offset);
	ret = table_value;

	kfree( fat_table );

	kprintf( "[%s] Have next cluster at 0x%x\n", __func__, ret );
	return ret;
}

// TODO:
unsigned fatfs_get_next_cluster_fat32( fatfs_device_t *dev, unsigned cluster ){
	unsigned ret = 0;

	kprintf( "[%s] Got here, probably an error\n", __func__ );

	return ret;
}
