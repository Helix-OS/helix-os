#ifndef SOME_SLAB_THING
#define SOME_SLAB_THING
//#include <stdlib.h>
//#include <sys/mman.h>
//#include "region.h"
#include <base/stdint.h>
#include <base/mem/region.h>

//#define BLOCK_SIZE     16
#define BLOCK_SIZE     4
#define MAX_FREE_PAGES 8
//#define MAX_FREE_PAGES 1
#define ALIGN_BLOCKS   0

#define SLAB_64_BIT_MAP
//#define SLAB_32_BIT_MAP
//#define SLAB_16_BIT_MAP
//#define SLAB_8_BIT_MAP

#define TESTSIZE     100
#define TESTITER     10000
#define SHORT_TEST         0
#define SHORT_TEST_NO_FREE 0
#define LONGER_TEST        1
#define REVERSE_TEST       0
#define MALLOC_TEST        0

#ifdef SLAB_64_BIT_MAP
#define SLAB_MAX_BITS 64
typedef uint64_t slabmap_t;

#else
#ifdef SLAB_32_BIT_MAP
#define SLAB_MAX_BITS 32
typedef uint32_t slabmap_t;

#else
#ifdef SLAB_16_BIT_MAP
#define SLAB_MAX_BITS 16
typedef uint16_t slabmap_t;

#else
#ifdef SLAB_8_BIT_MAP
#define SLAB_MAX_BITS 8
typedef uint8_t slabmap_t;
#endif
#endif
#endif
#endif

#define DARK_MAGIC 0xabadc0de

typedef void (*slab_ctor)(void *block);
typedef void (*slab_dtor)(void *block);

typedef enum {
	SLAB_STATUS_FREE,
	SLAB_STATUS_PARTIAL,
	SLAB_STATUS_FULL,
} slab_stat_t;

typedef struct slab {
	struct slab *last;
	struct slab *next;
	struct slab_cache *cache;

	slabmap_t map;
	unsigned bits_free;
	unsigned magic;
	slab_stat_t status;
} slab_t;

typedef struct slab_cache {
	slab_t *free;
	slab_t *partial;
	slab_t *full;

	slab_ctor ctor;
	slab_dtor dtor;

	region_t *region;

	unsigned obj_size;
	unsigned pages_alloced;
	unsigned map_bits;
	unsigned block_size;

	unsigned free_pages;
	unsigned max_free;
} slab_cache_t;

void *slab_alloc( slab_cache_t *cache );
void slab_free( slab_cache_t *cache, void *ptr );
void slab_free_from_ptr( void *ptr );
unsigned slab_ptr_get_obj_size( void *ptr );
unsigned slab_ptr_get_block_size( void *ptr );
slab_cache_t *new_slab_cache( unsigned size, slab_ctor ctor, slab_dtor dtor, region_t *region );
slab_cache_t *new_slab_cache_at_addr( slab_cache_t *cache,
                                      unsigned size,
                                      slab_ctor ctor,
                                      slab_dtor dtor,
									  region_t *region );

void delete_slab_cache( slab_cache_t *cache );

#endif
