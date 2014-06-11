#include <base/mem/memmap.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>

/** \brief Creates a structure describing a block of memory.
 *
 *  @param start  The starting address, inclusive
 *  @param end    The ending address, noninclusive
 *  @param type   The type of the block, see \ref memmap.h
 *  @param perms  The permissions for the memory, see \ref memmap.h
 *  @return A new memmap_t structure with the appropriate values set.
 */
memmap_t *memmap_create( unsigned long start, unsigned long end,
		memmap_type_t type, memmap_perm_t perms ){
	memmap_t *ret;

	ret = knew( memmap_t );
	ret->start = start;
	ret->end = end;
	ret->type = type;
	ret->perms = perms;
	ret->references = 0;

	return ret;
}

/** \brief Frees a previously allocated memmap_t.
 *
 *  This function currently does not deallocate pages, although this behaviour may
 *  change in the future. For now, deallocate any pages needed before passing the map
 *  to this function.
 *
 *  @param map  The map to free.
 */
void memmap_free( memmap_t *map ){
	// TODO: Free pages if map->references == 0
	kfree( map );
}

/** \brief Checks if an address falls within the boundaries of the given map.
 *
 *  @param map      The map to test against.
 *  @param address  The address to check for in the map.
 *  @return True if the address is greater than or equal to the starting address
 *          and less than the end address, false otherwise.
 */
bool memmap_check( memmap_t *map, unsigned long address ){
	bool ret;

	ret = address >= map->start && address < map->end;

	return ret;
}
