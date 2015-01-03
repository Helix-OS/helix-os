#ifndef _helix_fatfs_module_h
#define _helix_fatfs_module_h
#include <base/stdint.h>
#include <base/vfs/vfs.h>
#include <base/datastructs/hashmap.h>

typedef enum {
	FAT_TYPE_NULL,
	FAT_TYPE_12 		= 12,
	FAT_TYPE_16 		= 16,
	FAT_TYPE_32 		= 32,
} fat_type_t;

typedef enum {
	FAT_ATTR_NULL,
	FAT_ATTR_READ_ONLY 	= 1,
	FAT_ATTR_HIDDEN 	= 2,
	FAT_ATTR_SYSTEM 	= 4,
	FAT_ATTR_VOLUME_ID 	= 8,
	FAT_ATTR_LONGNAME 	= 0x0f,
	FAT_ATTR_DIRECTORY 	= 0x10,
	FAT_ATTR_ARCHIVE 	= 0X20,
} fat_attr_t;

enum {
	FAT_CLUSTER_BAD = 0xff7,
	FAT_CLUSTER_END = 0xff8,
};

// BIOS Parameter Block
typedef struct fatfs_bpb {
	uint8_t 	jumpcode[3];
	uint8_t 	oem_ident[8];
	uint16_t 	bytes_per_sect;
	uint8_t 	sect_per_clus;
	uint16_t 	reserved_sects;
	uint8_t 	fats;
	uint16_t 	dirents;
	uint16_t 	sectors;
	uint8_t 	media_desc;
	uint16_t 	sects_per_fat;
	uint16_t 	sects_per_track;
	uint16_t 	heads;
	uint32_t 	hidden_sects;
	uint32_t 	large_sectors;
} __attribute__((packed)) fatfs_bpb_t;

typedef struct fatfs_12_ebr {
	fatfs_bpb_t 	bpb;
	uint8_t 	drive_num;
	uint8_t 	nt_flags;
	uint8_t 	signature;
	uint32_t 	volume_id;
	uint8_t 	volume_label[11];
	uint8_t 	sysident[8];
} __attribute__((packed)) fatfs_12_ebr;

typedef struct fatfs_32_ebr {
	fatfs_bpb_t 	bpb;
} __attribute__((packed)) fatfs_32_ebr;

typedef struct fatfs_dirent {
	uint8_t 	name[11];
	uint8_t 	attributes;
	uint8_t 	nt_reserved;
	uint8_t 	time_created_tenths;
	uint16_t 	time_created;
	uint16_t 	date_created;
	uint16_t 	date_accessed;
	uint16_t 	cluster_high;
	uint16_t 	time_modified;
	uint16_t 	date_modified;
	uint16_t 	cluster_low;
	uint32_t 	size;
} __attribute__((packed)) fatfs_dirent_t;

typedef struct fatfs_longname_ent {
	uint8_t 	name_index;
	uint16_t 	name_first[5];
	uint8_t 	attributes;
	uint8_t 	longent_type;
	uint8_t 	checksum;
	uint16_t 	name_second[6];
	uint16_t 	unused;
	uint16_t 	name_third[2];
} __attribute__((packed)) fatfs_longname_ent_t;

typedef struct fatfs_device {
	file_node_t 	device_node; 
	fat_type_t 	type;
	hashmap_t 	*inode_map;

	union {
		fatfs_bpb_t 	*bpb;
		fatfs_12_ebr 	*fat12;
		fatfs_32_ebr 	*fat32;
	};
} fatfs_device_t;

typedef struct fatfs_dircache {
	fatfs_dirent_t dir;
	unsigned references;
} fatfs_dircache_t;

fat_type_t fatfs_get_type( fatfs_bpb_t *bpb );
unsigned fatfs_relclus_to_sect( fatfs_device_t *dev, unsigned cluster );

unsigned fatfs_get_next_cluster( fatfs_device_t *dev, unsigned cluster );
unsigned fatfs_get_next_cluster_fat12( fatfs_device_t *dev, unsigned cluster );
unsigned fatfs_get_next_cluster_fat16( fatfs_device_t *dev, unsigned cluster );
unsigned fatfs_get_next_cluster_fat32( fatfs_device_t *dev, unsigned cluster );

int fatfs_vfs_lookup( struct file_node *node, struct file_node *buf, char *name, int flags );
int fatfs_vfs_readdir( struct file_node *node, struct dirent *dirp, int entry );
int fatfs_vfs_read( struct file_node *node, void *buf, unsigned long length, unsigned long offset );
int fatfs_vfs_open( struct file_node *node, char *path, int flags );
int fatfs_vfs_close( struct file_node *node, int flags );
int fatfs_vfs_get_info( struct file_node *node, struct file_info *buf );

char *fatfs_apply_longname( fatfs_longname_ent_t *longname, char *namebuf, int maxlen );

#endif
