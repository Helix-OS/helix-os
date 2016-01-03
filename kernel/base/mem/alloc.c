#ifndef _helix_alloc_c
#define _helix_alloc_c

#include <base/mem/alloc.h>
#include <base/string.h>
#include <base/tasking/semaphore.h>
#include <base/mem/salloc.h>
#include <base/mem/region.h>

unsigned int early_placement = 0;
extern unsigned int end;
extern unsigned int *kernel_dir;
//struct mheap *kheap = 0;

static semaphore_t heap_semaphore = 1;
sheap_t kernel_heap;
region_t kernel_heap_region;

void *kmalloc_early( int size, int align ){
	void *ret;
	if ( !early_placement )
		early_placement = ((unsigned)&end & 0xfffff000) + 0x1000;

	if ( align && early_placement & 0xfff ){
		early_placement &= 0xfffff000;
		early_placement += 0x1000;
	}

	ret = (void *)early_placement;
	early_placement += size;

	//kprintf( "[alloc] returning 0x%x, size: 0x%x, placement: 0x%x\n", ret, size, early_placement );
	return ret;
}

sheap_t *init_kernel_heap( unsigned *p_dir, unsigned start, unsigned size ){
	//memset( heap, 0, sizeof( mheap_t ));

	map_pages( p_dir, start, start + size, PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT );
	flush_tlb( );
	void *bitmap = kmalloc_early( size / PAGE_SIZE / 8, 0 );

	bitmap_region_init_at_addr( (void *)start,
	                            size / PAGE_SIZE,
	                            &kernel_heap_region,
	                            bitmap,
	                            p_dir );

	sheap_init_at_addr( &kernel_heap, &kernel_heap_region );

    return &kernel_heap;

	/*
	heap->blocks = (mblock_t *)start;
	heap->first_free = heap->blocks;
	heap->page_dir = p_dir;

	heap->blocks->size = (size + (size & (PAGE_SIZE-1)? PAGE_SIZE : 0));
	heap->npages = heap->blocks->size / PAGE_SIZE;
	heap->page_blocks = 4;

	heap->blocks->type = MBL_FREE;
	heap->blocks->prev = 0;
	heap->blocks->next = MBL_END;

	return heap;
	*/

	return NULL;
}


void *kmalloc( int size ){
	void *ret = (void *)0;

	enter_semaphore( &heap_semaphore );
	//ret = amalloc( kheap, size, 0 );
	ret = sheap_alloc( &kernel_heap, size );
	leave_semaphore( &heap_semaphore );

	return ret;
}

void *kmalloca( int size ){
	void *ret;
	unsigned num_pages;

	enter_semaphore( &heap_semaphore );
	//ret = amalloc( kheap, size, 1 );
	//ret = sheap_alloc( &kernel_heap, size );
	num_pages = size / PAGE_SIZE + !!(size % PAGE_SIZE);
	//kprintf( "[%s] Got here, %u, %u\n", __func__, num_pages, size );
	//kprintf( "" );
	//unsigned foo;
	//for ( foo = 0; foo < 10000; foo ++ );
	ret = bitmap_region_alloc_page( &kernel_heap_region, num_pages );
	leave_semaphore( &heap_semaphore );

	return ret;
}

void *kcalloc( unsigned nmemb, unsigned memsize ){
	unsigned size;
	void *ret;

	size = nmemb * memsize;

	ret = kmalloc( size );
	if ( ret )
		memset( ret, 0, size );

	return ret;
}

void kfree( void *ptr ){
	enter_semaphore( &heap_semaphore );
	//afree( kheap, ptr );
	sheap_free( &kernel_heap, ptr );
	leave_semaphore( &heap_semaphore );
}

void kfreea( void *ptr ){
	enter_semaphore( &heap_semaphore );
	//afree( kheap, ptr );
	//sheap_free( &kernel_heap, ptr );
	bitmap_region_free_page( &kernel_heap_region, ptr );
	leave_semaphore( &heap_semaphore );
}

void *krealloc( void *ptr, unsigned long size ){
	void *ret = 0;

	enter_semaphore( &heap_semaphore );
	//ret = arealloc( kheap, ptr, size );
	ret = sheap_realloc( &kernel_heap, ptr, size );
	leave_semaphore( &heap_semaphore );

	return ret;
}

void kfree_early( void *ptr ){	
	return;
}

#endif
