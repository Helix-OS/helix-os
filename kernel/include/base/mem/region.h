#ifndef SOME_REGION_THING
#define SOME_REGION_THING 1
#include <base/stdint.h>
#include <arch/paging.h>

//#define PAGE_SIZE 0x1000
#define GLOBAL_REGION_SIZE        128 * 1024 * 1024
#define GLOBAL_REGION_NPAGES      GLOBAL_REGION_SIZE / PAGE_SIZE
#define GLOBAL_REGION_BITMAP_SIZE GLOBAL_REGION_NPAGES / 8

struct region;

typedef void *(*region_page_alloc)( struct region *region, unsigned n );
typedef void  (*region_page_free) ( struct region *region, void *ptr );

typedef struct region {
	void *addr;
	void *data;

	region_page_alloc alloc_page;
	region_page_free  free_page;

	unsigned pages;
	unsigned extra_data;
	unsigned *pagedir;
} region_t;

region_t *bitmap_region_init_at_addr( void *vaddr,
                                      unsigned size,
                                      region_t *addr,
                                      void *bitmap,
                                      unsigned *pagedir );

region_t *bitmap_region_init( void *vaddr, unsigned size, unsigned *pagedir );

void bitmap_region_free( region_t *addr );

void *bitmap_region_alloc_page( region_t *region, unsigned n );
void  bitmap_region_free_page( region_t *region, void *ptr );

#endif
