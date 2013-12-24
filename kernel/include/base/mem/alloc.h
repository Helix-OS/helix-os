#ifndef _helix_alloc_h
#define _helix_alloc_h
#include <base/arch/i386/paging.h>

#define MBL_FREE 0x66726565
#define MBL_USED 0x75736564
#define MBL_DIRTY 0x64697274
#define MBL_END  (struct mblock *)0xdeadbeef

typedef struct mblock {
	unsigned	type;
	unsigned	size;
	struct mblock 	*prev;
	struct mblock	*next;
} mblock_t;

typedef struct mheap {
	struct mblock *blocks;
	struct mblock *first_free;
	unsigned npages;
	unsigned page_blocks; // how many pages to allocate when increasing heap size
	unsigned *page_dir;
} mheap_t;

void *kmalloc_early( int size, int align );
void kfree_early( void *ptr );

void *kmalloc( int size );
void *kmalloca( int size );
void  kfree( void *ptr );
void *krealloc( void *ptr, unsigned long size );

mheap_t *init_heap( mheap_t *heap, unsigned *p_dir, unsigned start, unsigned size );

#endif
