#ifndef _helix_alloc_c
#define _helix_alloc_c
#include <base/mem/alloc.h>
#include <base/string.h>

extern unsigned int end;
static unsigned int early_placement = 0;
struct mheap *kheap = 0;
extern unsigned int *kernel_dir;

void *kmalloc_early( int size, int align ){
	void *ret;
	if ( !early_placement )
		early_placement = ((unsigned int)&end & 0xfffff000) + 0x1000;

	if ( align && early_placement & 0xfff ){
		early_placement &= 0xfffff000;
		early_placement += 0x1000;
	}

	ret = (void *)early_placement;
	early_placement += size;

	//kprintf( "[alloc] returning 0x%x, size: 0x%x, placement: 0x%x\n", ret, size, early_placement );
	return ret;
}

mheap_t *init_heap( mheap_t *heap, unsigned int *p_dir, unsigned int start, unsigned int size ){
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
	unsigned int	buf,
			tsize;
	mblock_t	*move,
			*temp,
			*blarg;

	if ( size < 16 ) size = 16;

	// include size of header
	size = size + sizeof( mblock_t );

	// round blocks up to the nearest block
	if ( size % sizeof( mblock_t ))
		size += (size + sizeof( mblock_t )) % sizeof( mblock_t ); 

	kprintf( "Finding block of size 0x%x\n", size );

	move = heap->first_free;
	while ( !found ){
		if ( move->type == MBL_FREE && move->size >= size && !align ){
			kprintf( "Found free block, size=0x%x\n", move->size );

			move->type = MBL_USED;
			if ( move->size > size && move->size - size > sizeof( mblock_t )){
				temp = (mblock_t *)((unsigned int)move + size);
				temp->size = move->size - size;
				temp->next = move->next;
				temp->prev = move;
				temp->type = MBL_FREE;

				heap->first_free = temp;

				move->size = size;
				move->next = temp;
				kprintf( "Splitting block, move->next=0x%x\n", move->next );
			}

			ret = (mblock_t *)((unsigned int)move + sizeof( mblock_t ));
			break;

		} else if ( align && move->type == MBL_FREE &&
			size < move->size - (PAGE_SIZE - (((unsigned int)move % PAGE_SIZE)?(unsigned int)move % PAGE_SIZE: PAGE_SIZE ) - sizeof( mblock_t ))){

			tsize = PAGE_SIZE - (((unsigned int)move % PAGE_SIZE)?(unsigned int)move % PAGE_SIZE: PAGE_SIZE ) - sizeof( mblock_t );
			temp = (mblock_t *)((unsigned int)move + tsize);

			if ( temp->type == MBL_USED ){
				kprintf( "This ain't no good thang\n" );
			} else {

				temp->type = MBL_USED;

				kprintf( "temp block: 0x%x, size: 0x%x\n", temp, tsize );

				if ( tsize ){
					temp->prev = move;
					temp->next = move->next;
					temp->size = move->size - tsize;

					move->next = temp;
					move->size = tsize;
					kprintf( "Split aligned block backwards\n" );
				}

				if ( temp->size > size && temp->size - size > sizeof( mblock_t )){
					blarg = (mblock_t *)((unsigned int)temp + size);
					blarg->size = temp->size - size;
					blarg->next = temp->next;
					blarg->prev = temp;
					blarg->type = MBL_FREE;

					if ( !tsize && blarg > heap->first_free )
						heap->first_free = blarg;

					temp->size = size;
					temp->next = blarg;
					kprintf( "Split aligned block forwards\n" );
				}

				ret = (mblock_t *)((unsigned int)temp + sizeof( mblock_t ));

				break;
			}

		} else if ( move->next == MBL_END ){
			kprintf( "Getting more pages...\n" );
			buf = heap->blocks + heap->npages * PAGE_SIZE;
			map_pages( heap->page_dir, buf, buf + heap->page_blocks * PAGE_SIZE, PAGE_WRITEABLE | PAGE_PRESENT );

			temp = (mblock_t *)buf;
			temp->prev = move;
			temp->size = heap->page_blocks * PAGE_SIZE;
			temp->type = MBL_FREE;
			temp->next = MBL_END;
			move->next = temp;
		}

		kprintf( "Trying next block...\n" );
		move = move->next;
	}

	return ret;
}

void afree( mheap_t *heap, void *ptr ){
	mblock_t	*move;

	move = (mblock_t *)((unsigned int)ptr - sizeof( mblock_t ));
	if ( move->type == MBL_USED ){
		kprintf( "Freeing 0x%x, size=0x%x\n", move, move->size );
		move->type = MBL_FREE;

		if ( move->next != MBL_END && move->next->type == MBL_FREE ){
			move->size += move->next->size;
			move->next = move->next->next;
			kprintf( "Joining next block, size=0x%x\n", move->size );
		}

		if ( move->prev && move->prev->type == MBL_FREE ){
			move->prev->size += move->size;
			move->prev->next = move->next;
			move = move->prev;
			kprintf( "Joining previous block, size=0x%x \n", move->size );
		}

		if ( move < heap->first_free )
			heap->first_free = move;
	}
}

void *kmalloc( int size, int align ){
	void *ret = (void *)0;

	ret = amalloc( kheap, size, align );
	
	return ret;
}

void kfree( void *ptr ){
	afree( kheap, ptr );
}

void kfree_early( void *ptr ){	
	return;
}

#endif
