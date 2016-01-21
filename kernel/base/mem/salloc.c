//#include <stdio.h>
#include <base/mem/salloc.h>
#include <base/mem/region.h>
#include <base/mem/slabnew.h>
#include <base/lib/stdbool.h>
//#include <string.h>
#include <base/string.h>
#include <base/logger.h>

static inline bool sheap_valid_denom( unsigned i ){
	unsigned minsize = PAGE_SIZE / SLAB_MAX_BITS;
	bool ret =
		((SLAB_MAX_BITS / i) % 2 != 1 )
		&& (PAGE_SIZE / (SLAB_MAX_BITS / i) / minsize) == i;

	return ret;
}

sheap_t *sheap_init_at_addr( void *addr, region_t *region ){
	sheap_t *ret = addr;

	unsigned minsize = PAGE_SIZE / SLAB_MAX_BITS;
	unsigned maxbits = 1024 / minsize;
	unsigned i = 0;

	ret->max_size = 1024;
	ret->region = region;

	for ( i = 1; i <= maxbits; i++ ){
		if ( sheap_valid_denom( i )) {
			new_slab_cache_at_addr( &ret->caches[i - 1], minsize * i, NULL, NULL, region );
			//printf( "Allocating cache at bit %u, %p\n", i, &ret->caches[i - 1] );
		}
	}

	return ret;
}

sheap_t *sheap_init( region_t *region ){
	return sheap_init_at_addr( kcalloc( 1, sizeof( sheap_t )), region );
}

void *sheap_alloc( sheap_t *heap, unsigned size ){
	void *ret = NULL;

	if ( !size ){
		size = 1;
	}

	// allocate small (less than 1/4th of the page size) blocks
	// from the array of slab allocators
	if ( size < heap->max_size ){
		unsigned bits;
		unsigned temp = PAGE_SIZE / SLAB_MAX_BITS;

		bits = ((size / temp) + (size % temp > 0));
		while ( !sheap_valid_denom( bits ) && bits <= 16 )
			bits++;

		//printf( "allocating, %u has %u bits\n", size, bits );

		if ( bits && bits <= 16 ){
			ret = slab_alloc( &heap->caches[bits - 1] );
		}

	// and just map pages for larger blocks, including the length of the
	// allocation in the first 4 bytes allocated
	} else {
		unsigned realsize = (size + sizeof(uint32_t));
		unsigned npages = realsize / PAGE_SIZE + !!(realsize % PAGE_SIZE);

		kprintf( "[%s] Allocating %u pages for size %u, real size %u\n",
			__func__, npages, size, realsize );

		ret = heap->region->alloc_page( heap->region, npages );

		*(uint32_t *)ret = size + sizeof(uint32_t);
		ret = (void *)((uintptr_t)ret + sizeof(uint32_t));
	}

	return ret;
}

void sheap_free( sheap_t *heap, void *ptr ){
	uintptr_t temp = (uintptr_t)ptr;
	
	if ( ptr ){
		// pass slab-allocated blocks to the slab's free
		if (( temp & (PAGE_SIZE - 1)) != 0x004 ){
			slab_free_from_ptr( ptr );

		// and unmap the pages mapped for larger blocks
		} else {
			void *pageptr = (void *)(temp & ~(PAGE_SIZE - 1));
			unsigned size = *(uint32_t *)pageptr;
			unsigned npages = (size / PAGE_SIZE) + (size % PAGE_SIZE > 0);
			unsigned i;

			kprintf( "got here, %p\n", pageptr );

			for ( i = 0; i < npages; i++ ){
				heap->region->free_page( heap->region, pageptr );
				pageptr = (void *)((uintptr_t)pageptr + PAGE_SIZE);
			}
		}
	}
}

static inline bool is_mapped_alloc( void *ptr ){
	uintptr_t temp = (uintptr_t)ptr;
	return ((temp & (PAGE_SIZE - 1)) == 0x004);
}

static inline uint32_t mapped_alloc_size( void *ptr ){
	return *(uint32_t *)ptr;
}

void *sheap_realloc( sheap_t *heap, void *ptr, unsigned size ){
	void *ret = NULL;

	// just return the passed ptr if the requested size is <=
	// the slab block size ptr is allocated in
	if ( !is_mapped_alloc( ptr ) && size <= slab_ptr_get_block_size( ptr )) {
		ret = ptr;

	// otherwise allocate a new piece of memory and copy things over
	} else if ( size > 0 ) {
		ret = sheap_alloc( heap, size );

		if ( ret ){
			unsigned obj_size;

			obj_size = is_mapped_alloc( ptr )
			         ? mapped_alloc_size( ptr )
			         : slab_ptr_get_obj_size( ptr );

			memcpy( ret, ptr, (obj_size < size)? obj_size : size );
		}

	} else {
		ret = sheap_alloc( heap, size );
	}

	return ret;
}

void *sheap_calloc( sheap_t *heap, unsigned nmemb, unsigned size ){
	void *ret = sheap_alloc( heap, nmemb * size );

	if ( ret ){
		memset( ret, 0, nmemb * size );
	}

	return ret;
}

/*
sheap_t __sheap_global_heap;
bool __sheap_initialized = false;
volatile bool __sheap_is_locked = false;

static inline void __sheap_lock( void ){
	__sheap_is_locked = true;
}

static inline void __sheap_unlock( void ){
	__sheap_is_locked = false;
}

static inline void __sheap_spinlock( void ){
	//while ( __sheap_is_locked ) puts( "LOCKED MAN" );
	while ( __sheap_is_locked );
}

static region_t __global_region;
static uint8_t __region_global_bitmap[GLOBAL_REGION_BITMAP_SIZE];

static inline void __sheap_global_init( void ){
	if ( !__sheap_initialized ){
		region_t *region = &__global_region;

		bitmap_region_init_at_addr(
								   (void *)0x9f00000000,
									GLOBAL_REGION_NPAGES,
									region,
									&__region_global_bitmap );

		void *mapret =
			mmap( (void *)0x9f00000000, GLOBAL_REGION_SIZE, 
				PROT_READ | PROT_WRITE,
				MAP_ANONYMOUS | MAP_PRIVATE,
				-1, 0 );

		if ( !mapret ){
			printf( "Could not map address, %p\n", mapret );
			exit( 1 );
		}

		memset( &__sheap_global_heap, 0, sizeof( sheap_t ));
		sheap_init_at_addr( &__sheap_global_heap, region );

		__sheap_initialized = true;
	}
}

void *malloc( size_t size ){
	void *ret = NULL;

	__sheap_spinlock( );
	__sheap_lock( );

	__sheap_global_init( );

	//printf( "[%s] got here, size %u\n", __func__, size );

	ret = sheap_alloc( &__sheap_global_heap, size );

	__sheap_unlock( );

	return ret;
}

void free( void *ptr ){
	//printf( "[%s] got here, ptr %p\n", __func__, ptr );

	__sheap_spinlock( );
	__sheap_lock( );

	if ( __sheap_initialized ){
		sheap_free( &__sheap_global_heap, ptr );
	}

	__sheap_unlock( );
}

void *realloc( void *ptr, size_t size ){
	void *ret = NULL;

	//printf( "[%s] got here, ptr %p, size %u\n", __func__, ptr, size );

	if ( ptr ){
		__sheap_spinlock( );
		__sheap_lock( );
		__sheap_global_init( );

		ret = sheap_realloc( &__sheap_global_heap, ptr, size );
		__sheap_unlock( );

	} else {
		ret = malloc( size );
	}

	return ret;
}

void *calloc( size_t nmemb, size_t size ){
	void *ret = NULL;
	__sheap_spinlock( );
	__sheap_lock( );
	__sheap_global_init( );

	ret = sheap_calloc( &__sheap_global_heap, nmemb, size );

	__sheap_unlock( );

	return ret;
}
*/

/*
#define FOO 2048
//#define FOO 48
#define BAR 2
#define BAZ 10
int main( int argc, char *argv[] ){
	sheap_t *foo = sheap_init( );
	void *bar[BAZ];
	unsigned i, k;
	
	for ( k = 0; k < BAR; k++ ){
		for ( i = 0; i < BAZ; i++ ){
			bar[i] = sheap_alloc( foo, FOO );
			printf( "got ptr at %p\n", bar[i] );
		}

		for ( i = 0; i < BAZ; i++ ){
			sheap_free( bar[i] );
		}
	}

	return 0;
}
*/
