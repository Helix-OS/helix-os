#ifndef _kernel_multiboot_h
#define _kernel_multiboot_h
#include <base/stdint.h>

enum {
	MULTIBOOT_FLAG_MEM	= 0x001,
	MULTIBOOT_FLAG_DEVICE	= 0x002,
	MULTIBOOT_FLAG_CMDLINE	= 0x004,
	MULTIBOOT_FLAG_MODS	= 0x008,
	MULTIBOOT_FLAG_AOUT	= 0x010,
	MULTIBOOT_FLAG_ELF	= 0x020,
	MULTIBOOT_FLAG_MMAP	= 0x040,
	MULTIBOOT_FLAG_CONFIG	= 0x080,
	MULTIBOOT_FLAG_LOADER	= 0x100,
	MULTIBOOT_FLAG_APM	= 0x200,
	MULTIBOOT_FLAG_VBE	= 0x400
};

enum { 
	MULTIBOOT_MEM_AVAIL	= 1,
	MULTIBOOT_MEM_RESERVE 	= 2
};

typedef struct multiboot_mem_map {
	uint32_t size;
	uint32_t addr_high;
	uint32_t addr_low;
	uint32_t len_high;
	uint32_t len_low;
	uint32_t type;
} __attribute__((packed)) multiboot_mem_map_t;

typedef struct multiboot_elf_sheaders {
	uint32_t num;
	uint32_t size;
	uint32_t addr;
	uint32_t shndx;
} __attribute__((packed)) multiboot_elf_t;

typedef struct multiboot_header {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	
	/*
	uint32_t num;
	uint32_t size;
	uint32_t addr;
	uint32_t shndx;
	*/
	multiboot_elf_t elf_headers;
	uint32_t mmap_length;
	uint32_t mmap_addr;
	uint32_t drives_length;
	uint32_t drives_addr;
	uint32_t config_table;
	uint32_t boot_loader_name;
	uint32_t apm_table;
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint32_t vbe_mode;
	uint32_t vbe_interface_seg;
	uint32_t vbe_interface_off;
	uint32_t vbe_interface_len;
} __attribute__((packed)) multiboot_header_t;
	
#endif
