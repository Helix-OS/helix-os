#ifndef _helix_fatfs_module_h
#define _helix_fatfs_module_h
#include <base/stdint.h>
#include <vfs/vfs.h>

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
	// tbd
} __attribute__((packed)) fatfs_32_ebr;

typedef struct fatfs_device {
	file_node_t 	device_node; 

	fatfs_bpb_t 	*bpb;
} fatfs_device_t;

#endif
