#ifndef _helix_alloc_h
#define _helix_alloc_h
#include <base/arch/i386/paging.h>

#define MBL_FREE 0x66726565
#define MBL_USED 0x75736564
#define MBL_END  (struct mblock *)0xdeadbeef

typedef struct mblock {
	unsigned int	type;
	unsigned int	size;
	struct mblock 	*prev;
	struct mblock	*next;
} mblock_t;

typedef struct mheap {
	struct mblock *blocks;
	struct mblock *first_free;
	unsigned int npages;
	unsigned int page_blocks; // how many pages to allocate when increasing heap size
	unsigned int *page_dir;
} mheap_t;

void *kmalloc_early( int size, int align );
void *kmalloc( int size, int align );
void kfree_early( void *ptr );

#endif
