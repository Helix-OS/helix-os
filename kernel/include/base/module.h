#ifndef _helix_module_h
#define _helix_module_h
#include <base/stdint.h>
#include <base/elf.h>

typedef struct mtable_header {
	uint32_t entries;
	uint32_t entry_size;
	uint32_t table_offset;
	uint32_t magic;
} mhead_t;

typedef struct mod_table {
	uint32_t offset;
	uint32_t len;
	uint32_t checksum;
	uint32_t magic;
} mtable_t;

void init_module_system( );
void load_init_modules( mhead_t *initmods );
int load_module( Elf32_Ehdr *elf_obj );

#endif
