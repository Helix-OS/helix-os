#ifndef _helix_mem_memmap_h
#define _helix_mem_memmap_h
#include <base/datastructs/list.h>
#include <base/lib/stdbool.h>

/** The possible fields for "type" in \ref memmap_t, describing
 *  what the memory is used for. */
typedef enum {
	MEMMAP_TYPE_NULL,
	MEMMAP_TYPE_KERNEL,
	MEMMAP_TYPE_IMAGE,
	MEMMAP_TYPE_USER,
} memmap_type_t;

/** The possible fields for "perms" in \ref memmap_t, describing
 *  the permissions of the memory. */
typedef enum {
	MEMMAP_PERM_NULL 	= 0,
	MEMMAP_PERM_READ 	= 1,
	MEMMAP_PERM_WRITE 	= 2,
	MEMMAP_PERM_EXEC 	= 4,
	MEMMAP_PERM_USER 	= 8,
} memmap_perm_t;

/** The memmap_t structure, used to describe a block of memory, and used particularly
 *  in paging (see \ref paging.c) code for on-demand memory allocation. */
typedef struct memmap {
	memmap_type_t type;
	memmap_perm_t perms;
	unsigned long start;
	unsigned long end;
	unsigned long references;
} memmap_t;

memmap_t *memmap_create( unsigned long start, unsigned long end,
		memmap_type_t type, memmap_perm_t perms );
void memmap_free( memmap_t *map );
bool memmap_check( memmap_t *map, unsigned long address );

#endif
