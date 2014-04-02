#ifndef _helix_alloc_c
#define _helix_alloc_c

#include <base/mem/alloc.h>
#include <base/string.h>
#include <base/tasking/semaphore.h>

unsigned int early_placement = 0;
extern unsigned int end;
extern unsigned int *kernel_dir;
struct mheap *kheap = 0;

static semaphore_t heap_semaphore = 1;

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

mheap_t *init_heap( mheap_t *heap, unsigned *p_dir, unsigned start, unsigned size ){
	memset( heap, 0, sizeof( mheap_t ));

	map_pages( p_dir, start, start + size, PAGE_WRITEABLE | PAGE_PRESENT );
	flush_tlb( );
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
}

void *amalloc( mheap_t *heap, int size, int align ){
	void *ret = 0;
	int found = 0;
	unsigned	buf,
			tsize;
	mblock_t	*move,
			*temp,
			*blarg;

	if ( size < sizeof( mblock_t )) size = sizeof( mblock_t );

	// include size of header
	size = size + sizeof( mblock_t );

	// round blocks up to the nearest block
	size += (sizeof( mblock_t ) - size % sizeof( mblock_t )) % sizeof( mblock_t );

	//kprintf( "Finding block of size 0x%x\n", size );

	move = heap->first_free;
	while ( !found ){
		if ( move->type == MBL_FREE && move->size >= size && !align ){
			//kprintf( "Found free block, size=0x%x\n", move->size );

			move->type = MBL_USED;
			if ( move->size > size && move->size - size > sizeof( mblock_t )){
				temp = (mblock_t *)((unsigned)move + size);
				temp->size = move->size - size;
				temp->next = move->next;
				temp->prev = move;
				temp->type = MBL_FREE;

				if ( heap->first_free->type == MBL_USED || temp < heap->first_free )
					heap->first_free = temp;

				move->size = size;
				move->next = temp;
				//kprintf( "Splitting block, move->next=0x%x\n", move->next );
			}

			ret = (mblock_t *)((unsigned)move + sizeof( mblock_t ));
			break;

		} else if ( align && move->type == MBL_FREE &&
			size < move->size - (PAGE_SIZE - (unsigned)move % PAGE_SIZE) - sizeof( mblock_t )){

			tsize = PAGE_SIZE - ((unsigned)move % PAGE_SIZE) - sizeof( mblock_t );

			temp = (mblock_t *)((unsigned)move + tsize);
			//kprintf( "tsize: 0x%x, ", tsize );

			if ( tsize > move->size || temp->type == MBL_USED ){
				//kprintf( "This ain't no good thang\n" );
			} else {

				temp->type = MBL_USED;

				//kprintf( "temp block: 0x%x, size: 0x%x\n", temp, tsize );

				if ( tsize ){
					temp->prev = move;
					temp->next = move->next;
					temp->size = move->size - tsize;

					move->next = temp;
					move->size = tsize;
					//kprintf( "Split aligned block backwards\n" );
				}

				if ( temp->size > size && temp->size - size > sizeof( mblock_t )){
					blarg = (mblock_t *)((unsigned)temp + size);
					blarg->size = temp->size - size;
					blarg->next = temp->next;
					blarg->prev = temp;
					blarg->type = MBL_FREE;

					if ( !tsize && blarg < heap->first_free )
						heap->first_free = blarg;

					temp->size = size;
					temp->next = blarg;
					//kprintf( "Split aligned block forwards\n" );
				}

				ret = (mblock_t *)((unsigned)temp + sizeof( mblock_t ));

				break;
			}

		} else if ( move->next == MBL_END ){
			//kprintf( "Getting more pages...\n" );
			buf = (unsigned)heap->blocks + heap->npages * PAGE_SIZE;
			map_pages( heap->page_dir, buf, buf + heap->page_blocks * PAGE_SIZE, PAGE_WRITEABLE | PAGE_PRESENT );

			heap->npages += heap->page_blocks;

			temp = (mblock_t *)buf;
			temp->prev = move;
			temp->size = heap->page_blocks * PAGE_SIZE;
			temp->type = MBL_FREE;
			temp->next = MBL_END;
			move->next = temp;
		}

		//kprintf( "Have block with size 0x%x, trying next block...\n", move->size );
		move = move->next;
	}

	return ret;
}

void afree( mheap_t *heap, void *ptr ){
	mblock_t	*move;

	if ( !ptr ){
		kprintf( "[afree] Warning! Caught null pointer, bailing out now\n" );
		return;
	}

	move = (mblock_t *)((unsigned)ptr - sizeof( mblock_t ));
	if ( move->type == MBL_USED ){
		/*
		 kprintf( "Freeing 0x%x, size=0x%x, next=0x%x, prev=0x%x\n",
			move, move->size, move->next, move->prev );
		*/
		move->type = MBL_FREE;

		while ( move->next != MBL_END && move->next->type == MBL_FREE ){
			//kprintf( "next... " );
			move->size += move->next->size;
			move->next->type = MBL_DIRTY;
			move->next = move->next->next;
			/*
			kprintf( "Joining next block 0x%x(%d), size=0x%x\n",
				move->next, move->next == MBL_END, move->size );
			*/
		}

		while ( move->prev && move->prev->type == MBL_FREE ){
			//kprintf( "Previous... " );
			move->prev->size += move->size;
			move->prev->next = move->next;
			move = move->prev;

			if ( move->next != MBL_END )
				move->next->prev = move;

			//kprintf( "Joining previous block, size=0x%x \n", move->size );
		}

		if ( move < heap->first_free )
			heap->first_free = move;
	}
}

void *arealloc( mheap_t *heap, void *ptr, unsigned long size ){
	void 		*ret = 0,
			*buf;
	mblock_t 	*move,
			*temp;
	unsigned long 	nsize;

	if ( !ptr ){
		kprintf( "[arealloc] Warning! Caught null pointer, bailing out now\n" );
		return ptr;
	}

	move = (mblock_t *)((unsigned)ptr - sizeof( mblock_t ));
	nsize = size + ((sizeof( mblock_t ) - size ) % sizeof( mblock_t )) % sizeof( mblock_t );

	if ( move->type == MBL_USED ){
		if ( nsize > move->size - sizeof( mblock_t )){
			if ( move->next != MBL_END && move->next->type == MBL_FREE &&
					move->size + move->next->size > nsize + sizeof( mblock_t )){

				nsize += sizeof( mblock_t );
				temp = (mblock_t *)((unsigned long)move->next + ( nsize - move->size ));
				temp->next = move->next->next;
				temp->size = move->next->size - ( nsize - move->size );
				temp->type = MBL_FREE;
				temp->prev = move;

				/*
				kprintf( "[arealloc] Split next block forwards: 0x%x, 0x%x, 0x%x, 0x%x\n",
						temp, temp->size, temp->prev, temp->next );
				*/

				if ( heap->first_free == move->next )
					heap->first_free = temp;

				move->next = temp;
				move->size = nsize;

				ret = ptr;
			} else {
				//kprintf( "[arealloc] Allocating new block\n" );
				buf = amalloc( heap, nsize, 0 );
				if ( ptr ){
					memcpy( buf, ptr, move->size );
					afree( heap, ptr );
				}

				ret = buf;
			}
		} else {
			ret = ptr;
		}
	}

	return ret;
}

void *kmalloc( int size ){
	void *ret = (void *)0;

	enter_semaphore( &heap_semaphore );
	ret = amalloc( kheap, size, 0 );
	leave_semaphore( &heap_semaphore );

	return ret;
}

void *kmalloca( int size ){
	void *ret;

	enter_semaphore( &heap_semaphore );
	ret = amalloc( kheap, size, 1 );
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
	afree( kheap, ptr );
	leave_semaphore( &heap_semaphore );
}

void *krealloc( void *ptr, unsigned long size ){
	void *ret = 0;

	enter_semaphore( &heap_semaphore );
	ret = arealloc( kheap, ptr, size );
	leave_semaphore( &heap_semaphore );

	return ret;
}

void kfree_early( void *ptr ){	
	return;
}

void dump_aheap_blocks( mheap_t *heap ){
	char *status[] = { 	"free",
				"used" };
	mblock_t *move = heap->blocks;
	int i;

	if ( !heap )
		return;

	kprintf( "+----------------[ Heap dump ]---------------+\n" );
	for ( i = 0; move && move != MBL_END; move = move->next, i++ ){
		kprintf( "| 0x%x: size: 0x%x\tprev: 0x%x\tnext: 0x%x\tstatus: %s |\n",
			move, move->size, move->prev, move->next, status[ move->type == MBL_USED ]);
	}
	kprintf( "| %d blocks, %d pages\t\t\t     |\n", i, heap->npages );
	kprintf( "+--------------------------------------------+\n" );
}

#endif
