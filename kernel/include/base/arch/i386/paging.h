#ifndef _helix_paging_h
#define _helix_paging_h
#include <base/arch/i386/isr.h>

#define PAGE_USER	4
#define PAGE_WRITEABLE	2
#define PAGE_PRESENT	1
#define PAGE_SIZE	0x1000

typedef unsigned page_dir_t;

int map_page(	unsigned *dir, unsigned vaddress );
int map_r_page( unsigned *dir, unsigned vaddress, unsigned raddress );
int map_pages(	unsigned *dir, unsigned start, unsigned end, unsigned permissions );
int remap_pages( unsigned *dir, unsigned start, unsigned end, unsigned permissions );
int free_page(	unsigned *dir, unsigned vaddress );
int set_page(	unsigned *dir, unsigned vaddress );
unsigned get_page( unsigned *dir, unsigned vaddress );

void set_page_dir( unsigned *dir );
void flush_tlb( );
unsigned *clone_page_dir( unsigned *dir );
page_dir_t *get_current_page_dir( );
page_dir_t *get_kernel_page_dir( );

int init_paging( unsigned max_mem );
void page_fault_handler( registers_t *regs );

unsigned get_free_page( );

#endif
