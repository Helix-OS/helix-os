/*
Copyright (c) 2015, defun
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//#include <stdio.h>
#include <base/string.h>
#include <base/lib/stdbool.h>
#include <base/mem/slabnew.h>

void dump_cache_blocks( slab_cache_t *cache, slab_t *slab ){
	if ( slab ){
		uint64_t i;
		char *statusstr[] = { "free", "partial", "full" };

		kprintf( "%s slab %p:\n", statusstr[slab->status], slab );
		kprintf( "    bitmap: " );
		for ( i = 0; i < cache->map_bits; i++ ){
			//putchar( (slab->map & ((slabmap_t)1 << i))? '1' : '0' );
			kprintf( "%c", (slab->map & ((slabmap_t)1 << i))? '1' : '0' );
		}
		//putchar( '\n' );
		kprintf( "\n" );
		kprintf( "    %u bits free\n", slab->bits_free );

		dump_cache_blocks( cache, slab->last );
	}
}

static inline void *slab_cache_alloc_pages( slab_cache_t *cache, unsigned n ){
	void *ret = cache->region->alloc_page( cache->region, n );

	return ret;
}

static inline void slab_cache_push_block( slab_t **slist, slab_t *slab ){
	// add mutex here for threading
	slab->last = *slist;

	if (*slist){
		(*slist)->next = slab;
	}

	slab->next = NULL;
	*slist = slab;
}

static inline void slab_cache_move_block( slab_t **from, slab_t **to, slab_t *slab ){
	// and here
	if ( slab->last ){
		slab->last->next = slab->next;
	}

	*(slab->next? &slab->next->last : from) = slab->last;
	slab->next = NULL;
	slab->last = *to;

	if (*to){
		(*to)->next = slab;
	}

	*to = slab;
}

// could be better named?
slab_cache_t *slab_cache_add_page( slab_cache_t *cache, void *page ){
	slab_cache_t *ret = cache;
	uintptr_t pos = (uintptr_t)page;
	slab_t *slab = page;
	unsigned i;

	slab->map = 1;
	slab->status = SLAB_STATUS_FREE;
	slab->bits_free = cache->map_bits - 1;
	slab->magic = DARK_MAGIC;
	slab->last = slab->next = NULL;
	slab->cache = cache;

	if ( cache->ctor ){
		for ( i = 1; i < cache->map_bits; i += 1 ){
			pos += cache->block_size;
			cache->ctor( (void *)pos );
		}
	}

	cache->free_pages++;
	cache->pages_alloced++;
	slab_cache_push_block( &cache->free, slab );

	return ret;
}

slab_cache_t *slab_cache_add_pages( slab_cache_t *cache, void *pages, unsigned n ){
	slab_cache_t *ret = cache;
	uintptr_t curpage = (uintptr_t)pages;
	unsigned i;

	for ( i = 0; i < n; i++ ){
		slab_cache_add_page( cache, (void *)curpage );
		curpage += PAGE_SIZE;
	}

	return ret;
}

void slab_cache_add_page_block( slab_cache_t *cache ){
	void *buf = slab_cache_alloc_pages( cache, BLOCK_SIZE );

	if ( buf ){
		slab_cache_add_pages( cache, buf, BLOCK_SIZE );
	}
}

void slab_cache_do_dtors( slab_cache_t *cache, void *page ){
	if ( cache->dtor ){
		unsigned i;
		uintptr_t pos = (uintptr_t)page;

		for ( i = 1; i < cache->map_bits; i += 1 ){
			pos += cache->block_size;
			cache->dtor( (void *)pos );
		}
	}
}

void free_one_cache_page( slab_cache_t *cache ){
	if ( cache->free ){
		slab_t *tmp = cache->free;
		cache->free = tmp->last;
		cache->pages_alloced--;
		cache->free_pages--;

		if ( tmp->next ) tmp->next->last = tmp->last;
		if ( tmp->last ) tmp->last->next = tmp->next;

		slab_cache_do_dtors( cache, tmp );
		cache->region->free_page( cache->region, tmp );
		//printf( "freed one slab at %p\n", tmp );
	}
}

void free_slab_list( slab_cache_t *cache, slab_t *slab ){
	slab_t *move, *temp;

	for ( move = slab; move; move = temp ){
		temp = move->last;
		slab_cache_do_dtors( cache, move );
		//munmap( move, PAGE_SIZE );
		cache->region->free_page( cache->region, move );
		cache->pages_alloced--;
	}
}

void *slab_alloc( slab_cache_t *cache ){
	void *ret = NULL;
	unsigned i;

	for ( i = 0; !ret && i < 2; i++ ){
		if ( cache->partial ){
			slabmap_t b;
			slab_t *slab = cache->partial;
			uintptr_t move = (uintptr_t)slab;

			for ( b = 1; slab->map & b; b <<= 1 ){
				move += cache->block_size;
			}

			ret = (void *)move;
			slab->map |= b;
			slab->bits_free -= 1;

			if ( slab->bits_free == 0 ){
				slab->status = SLAB_STATUS_FULL;
				slab_cache_move_block( &cache->partial, &cache->full, slab );
			}

		} else if ( cache->free ){
			slab_t *slab = cache->free;

			ret = (void *)((uintptr_t)slab + cache->block_size);

			slab->map |= 1 << 1;
			slab->status = SLAB_STATUS_PARTIAL;
			slab->bits_free -= 1;
			cache->free_pages--;

			slab_cache_move_block( &cache->free, &cache->partial, slab );
		}

		if ( !ret ) {
			slab_cache_add_page_block( cache );
		}
	}

	return ret;
}

void slab_free( slab_cache_t *cache, void *ptr ){
	uintptr_t temp = (uintptr_t)ptr;
	uintptr_t slabpos = (uintptr_t)ptr & ~(PAGE_SIZE - 1);
	slab_t *slab = (void *)slabpos;

	//printf( "freeing at block %p, with magick 0x%x\n", slab, slab->magic );

	if ( slab->magic == DARK_MAGIC ){
	    unsigned bits = (temp - slabpos) / cache->block_size;

		slab->map &= ~((slabmap_t)1 << bits);
	    slab->bits_free += 1;

	    if ( slab->status == SLAB_STATUS_FULL && slab->bits_free >= 1 ) {
			slab->status = SLAB_STATUS_PARTIAL;
			slab_cache_move_block( &cache->full, &cache->partial, slab );

	    } else if ( slab->status == SLAB_STATUS_PARTIAL
			&& slab->bits_free == cache->map_bits - 1 )
	    {
			slab->status = SLAB_STATUS_FREE;
			cache->free_pages++;
			slab_cache_move_block( &cache->partial, &cache->free, slab );
	    }

	} else {
		kprintf( "Warning: possibly corrupted or invalid slab block given at %p!\n", slab );
	}

	//while ( cache->free_pages > cache->max_free ){
	if ( cache->free_pages > cache->max_free ){
		free_one_cache_page( cache );
	}
}

void slab_free_from_ptr( void *ptr ){
	uintptr_t curpage = (uintptr_t)ptr & ~(PAGE_SIZE - 1);
	slab_t *slab = (void *)curpage;

	if ( slab->magic == DARK_MAGIC ){
		slab_free( slab->cache, ptr );
	}
}

unsigned slab_ptr_get_obj_size( void *ptr ){
	uintptr_t curpage = (uintptr_t)ptr & ~(PAGE_SIZE - 1);
	slab_t *slab = (void *)curpage;

	if ( slab->magic == DARK_MAGIC ){
		return slab->cache->obj_size;

	} else {
		return 0;
	}
}

unsigned slab_ptr_get_block_size( void *ptr ){
	uintptr_t curpage = (uintptr_t)ptr & ~(PAGE_SIZE - 1);
	slab_t *slab = (void *)curpage;

	if ( slab->magic == DARK_MAGIC ){
		return slab->cache->block_size;

	} else {
		return 0;
	}
}

slab_cache_t *new_slab_cache_at_addr( slab_cache_t *cache,
                                      unsigned size,
                                      slab_ctor ctor,
                                      slab_dtor dtor,
                                      region_t *region )
{
	if ( cache ){
		unsigned temp = PAGE_SIZE / SLAB_MAX_BITS;

		cache->free_pages = 0;
		cache->pages_alloced = 0;
		cache->max_free = MAX_FREE_PAGES;
		cache->obj_size = size;
		cache->free = cache->partial = cache->full = NULL;
		cache->ctor = ctor;
		cache->dtor = dtor;
		cache->region = region;

		cache->map_bits = SLAB_MAX_BITS / ((cache->obj_size / temp)
		                                + (cache->obj_size % temp > 0));
		cache->map_bits = cache->map_bits - (cache->map_bits % 2);
		cache->block_size = PAGE_SIZE / cache->map_bits;

#if ALIGN_BLOCKS
		if ( cache->map_bits > 8 ){
			cache->map_bits = (cache->map_bits / 8) * 8;
		}
#endif

		slab_cache_add_page_block( cache );
		kprintf( "Initialized, first cache block is at %p\n", cache->free );
	}

	return cache;
}

slab_cache_t *new_slab_cache( unsigned size, slab_ctor ctor, slab_dtor dtor, region_t *region ){
	slab_cache_t *cache = NULL;

	if (( cache = kcalloc( 1, sizeof( slab_cache_t )))){
		new_slab_cache_at_addr( cache, size, ctor, dtor, region );

	} else {
		//perror( "calloc()" );
		kprintf( "[%s] Could not allocate memory from kcalloc()\n", __func__ );
	}

	return cache;
}

void delete_slab_cache( slab_cache_t *cache ){
	free_slab_list( cache, cache->free );
	free_slab_list( cache, cache->partial );
	free_slab_list( cache, cache->full );
}
