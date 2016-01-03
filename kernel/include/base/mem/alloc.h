#ifndef _helix_alloc_h
#define _helix_alloc_h
#include <arch/paging.h>
#include <base/logger.h>

#ifdef __cplusplus
extern "C" {
#endif

void *kmalloc_early( int size, int align );
void kfree_early( void *ptr );

void *kmalloc( int size );
void *kmalloca( int size );
void  kfree( void *ptr );
void  kfreea( void *ptr );
void *krealloc( void *ptr, unsigned long size );
void *kcalloc( unsigned nmemb, unsigned size );

#ifdef __cplusplus
}
#endif

#endif
