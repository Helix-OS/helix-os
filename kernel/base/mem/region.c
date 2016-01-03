//#include "region.h"
#include <base/mem/region.h>
#include <base/mem/alloc.h>
//#include <stdlib.h>
#include <base/lib/stdbool.h>
//#include <stdio.h>

region_t *bitmap_region_init_at_addr( void *vaddr,
                                      unsigned npages,
                                      region_t *addr,
                                      void *bitmap,
                                      unsigned *pagedir )
{
	region_t *ret = addr;

	ret->pages = npages;
	ret->addr = vaddr;
	ret->data = bitmap;
	ret->alloc_page = bitmap_region_alloc_page;
	ret->free_page  = bitmap_region_free_page;
	ret->extra_data = 0;
	ret->pagedir = pagedir;

	return ret;
}

region_t *bitmap_region_init( void *vaddr, unsigned size, unsigned *pagedir ){
	region_t *ret = NULL;
	
	if (( ret = kcalloc( 1, sizeof( region_t )))) {
		unsigned npages = size / PAGE_SIZE + (size % PAGE_SIZE > 0);
		void *bitmap = kcalloc( 1, npages / 8 );

		if ( bitmap ){
			ret = bitmap_region_init_at_addr( vaddr, npages, ret, bitmap, pagedir );

		} else {
			kfree( ret );
			ret = NULL;
		}
	}

	return ret;
}

void bitmap_region_free( region_t *addr ){
}

static inline uint8_t bitmap_get( uint8_t *map, unsigned index ){
	return !!(map[index / 8] & (1 << (index % 8)));
}

static inline uint8_t bitmap_set( uint8_t *map, unsigned index ){
	return map[index / 8] |= (1 << (index % 8));
}

static inline uint8_t bitmap_unset( uint8_t *map, unsigned index ){
	return map[index / 8] &= ~(1 << (index % 8));
}

void *bitmap_region_alloc_page( region_t *region, unsigned n ){
	void *ret = NULL;
	uint8_t *bitmap = region->data;
	unsigned i;
	bool found = false;
	bool set_index = false;

	for ( i = region->extra_data; i < region->pages && !found; ) {
		if (( bitmap[i] & 0xff ) == 0xff ){
			i++;

		} else {
			unsigned first_free = i * 8;
			uintptr_t temp = (uintptr_t)region->addr;
			unsigned k;

			if ( !set_index ){
				region->extra_data = i;
				set_index = true;
			}

			for ( ; bitmap_get( bitmap, first_free ); first_free++ );

			if ( n > 1 ){
				bool is_free = true;

				for ( k = 0; k < n && is_free; k++ ){
					is_free = bitmap_get( bitmap, first_free + k ) == 0;
				}

				if ( !is_free ){
					i += 1;
					continue;

				} else {
					for ( k = 0; k < n; k++ ){
						bitmap_set( bitmap, first_free + k );
						// add map_pages call here
					}

					unsigned start = (uintptr_t)region->addr + PAGE_SIZE * first_free;
					unsigned end   = (uintptr_t)region->addr + PAGE_SIZE * (first_free + n);

					map_pages( region->pagedir, start, end, PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT );
					flush_tlb( );
				}

			} else {
				bitmap_set( bitmap, first_free );
				unsigned start = (uintptr_t)region->addr + PAGE_SIZE * first_free;
				map_page( region->pagedir, start, PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT );
				flush_tlb( );
				// also map_page here
			}

			temp += first_free * PAGE_SIZE;
			ret = (void *)temp;
			found = true;

			/*
			printf( "[%s] Found %u free bit(s) at %u, returning %p\n",
				__func__, n, first_free, ret );

			{
				unsigned k;
				printf( "[%s] bitmap stuff: ", __func__ );
				for ( k = 0; k < 16; k++ ){
					printf( "0x%02x ", bitmap[k] );
				}
			}

			printf( "\n" );

			printf( "[%s] First free at %u\n", __func__, region->extra_data );
			*/
		}
	}

	return ret;
}

void  bitmap_region_free_page( region_t *region, void *ptr ){
	uintptr_t vaddr = (uintptr_t)region->addr;
	uintptr_t temp  = (uintptr_t)ptr;
	unsigned index = (temp - vaddr) / PAGE_SIZE;
	unsigned bytepos = index % 8;

	bitmap_unset( region->data, index );

	if ( bytepos <= region->extra_data ){
		region->extra_data = bytepos;
	}
	// add free_page call here
	free_page( region->pagedir, temp );
	flush_tlb( );

	//printf( "[%s] Freeing ptr at %p, bit %u\n", __func__, ptr, index );
}
