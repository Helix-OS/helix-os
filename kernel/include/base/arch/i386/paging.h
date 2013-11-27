#ifndef _helix_paging_h
#define _helix_paging_h
#include <base/arch/i386/isr.h>

#define PAGE_USER	4
#define PAGE_WRITEABLE	2
#define PAGE_PRESENT	1
#define PAGE_SIZE	0x1000

int map_page(	unsigned int *dir, unsigned int vaddress );
int map_r_page( unsigned int *dir, unsigned int vaddress, unsigned int raddress );
int map_pages(	unsigned int *dir, unsigned int start, unsigned int end, unsigned int permissions );
int free_page(	unsigned int *dir, unsigned int vaddress );
int set_page(	unsigned int *dir, unsigned int vaddress );
unsigned int get_page( unsigned int *dir, unsigned int vaddress );

void set_page_dir( unsigned int *dir );
void flush_tlb( );
unsigned int *get_current_page_dir( );

int init_paging( unsigned int max_mem );
void page_fault_handler( registers_t *regs );

unsigned int get_free_page( );

#endif
