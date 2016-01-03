#ifndef _helix_salloc_h
#define _helix_salloc_h 1
#include <base/mem/slabnew.h>

typedef struct sheap {
	slab_cache_t caches[16];
	region_t *region;
	unsigned max_size;
} sheap_t;

sheap_t *sheap_init_at_addr( void *addr, region_t *region );
sheap_t *sheap_init( region_t *region );
void *sheap_alloc( sheap_t *heap, unsigned size );
void sheap_free( sheap_t *heap, void *ptr );
void *sheap_realloc( sheap_t *heap, void *ptr, unsigned size );
void *sheap_calloc( sheap_t *heap, unsigned nmemb, unsigned size );

sheap_t *init_kernel_heap( unsigned *p_dir, unsigned start, unsigned size );

#endif
