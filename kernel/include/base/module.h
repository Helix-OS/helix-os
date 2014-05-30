#ifndef _helix_module_h
#define _helix_module_h
#include <base/stdint.h>
#include <base/elf.h>
#include <base/multiboot.h>
#include <base/string.h>
#include <base/datastructs/list.h>
#include <base/datastructs/dlist.h>
#include <base/datastructs/hashmap.h>

// Some structs to access the initmods images
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

// Structs to keep track of modules
typedef struct module {
	char 		*name;
	Elf32_Ehdr 	*elf_head;
	unsigned long 	address;
	unsigned long 	pages;
	char 		*def_symtab;
	
	dlist_container_t *depends;
	dlist_container_t *links;

	hashmap_t 	*symcache;
	/*
	int 		ndeps;
	struct module 	*depends;
	*/

	/*
	int 		nlinks;
	struct module 	*links;
	*/

	//struct module 	*next;
} module_t;

void init_module_system( multiboot_elf_t *elfinfo );
void load_init_modules( mhead_t *initmods );
int load_module( Elf32_Ehdr *elf_obj );

#endif
