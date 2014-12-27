#ifndef _helix_x86_init_tables_c
#define _helix_arch_x86_init_tables_c

#include <arch/init_tables.h>
#include <base/stdio.h>
#include <base/string.h>

static void init_gdt( void );
static void init_idt( void );
static void gdt_set_gate( int32_t, uint32_t, uint32_t, uint8_t, uint8_t );
static void idt_set_gate( uint8_t, uint32_t, uint16_t, uint8_t );
static void write_tss( int32_t, uint16_t, uint32_t );
extern void gdt_flush( uint32_t );
extern void idt_flush( uint32_t );
extern void tss_flush( );

gdt_entry_t gdt_entries[6];
gdt_ptr_t   gdt_ptr;

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

tss_entry_t tss_entry;

void init_tables( void ){
	init_gdt();
	init_idt();
	memset( &interrupt_handlers, 0, sizeof( isr_t ) * 256 );
	register_interrupt_handler(  0x0, &zero_division_fault );
	register_interrupt_handler(  0x6, &invalid_op_fault );
	register_interrupt_handler(  0x8, &double_fault );
	register_interrupt_handler(  0xd, &gen_protect_fault );
	register_interrupt_handler( 0x30, &dump_registers );
}

void set_kernel_stack( uint32_t stack ){
	tss_entry.esp0 = stack;
}

static void init_gdt( void ){
	gdt_ptr.limit = sizeof(gdt_entry_t) * 6 - 1;
	gdt_ptr.base  = (uint32_t)&gdt_entries;

	gdt_set_gate( 0, 0, 0, 		0,    0    );
	gdt_set_gate( 1, 0, 0xffffffff, 0x9a, 0xcf );
	gdt_set_gate( 2, 0, 0xffffffff, 0x92, 0xcf );
	gdt_set_gate( 3, 0, 0xffffffff, 0xfa, 0xcf );
	gdt_set_gate( 4, 0, 0xffffffff, 0xf2, 0xcf );
	write_tss( 5, 0x10, 0 );

	gdt_flush(( uint32_t)&gdt_ptr );
	tss_flush();
}

static void init_idt( void ){
	idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
	idt_ptr.base  = (uint32_t)&idt_entries;

	memset( &idt_entries, 0, sizeof( idt_entry_t) * 256 );

	//remap irq table
	outb( 0x20, 0x11 );
	outb( 0xa0, 0x11 );
	outb( 0x21, 0x20 );
	outb( 0xa1, 0x28 );
	outb( 0x21, 0x04 );
	outb( 0xa1, 0x02 );
	outb( 0x21, 0x01 );
	outb( 0xa1, 0x01 );
	outb( 0x21, 0x0  );
	outb( 0xa1, 0x0  );

	//set needed stuffs
	idt_set_gate( 0, (uint32_t)isr0,   0x08, 0x8e );
	idt_set_gate( 1, (uint32_t)isr1,   0x08, 0x8e );
	idt_set_gate( 2, (uint32_t)isr2,   0x08, 0x8e );
	idt_set_gate( 3, (uint32_t)isr3,   0x08, 0x8e );
	idt_set_gate( 4, (uint32_t)isr4,   0x08, 0x8e );
	idt_set_gate( 5, (uint32_t)isr5,   0x08, 0x8e );
	idt_set_gate( 6, (uint32_t)isr6,   0x08, 0x8e );
	idt_set_gate( 7, (uint32_t)isr7,   0x08, 0x8e );
	idt_set_gate( 8, (uint32_t)isr8,   0x08, 0x8e );
	idt_set_gate( 9, (uint32_t)isr9,   0x08, 0x8e );
	idt_set_gate( 10,(uint32_t)isr10,  0x08, 0x8e );
	idt_set_gate( 11,(uint32_t)isr11,  0x08, 0x8e );
	idt_set_gate( 12,(uint32_t)isr12,  0x08, 0x8e );
	idt_set_gate( 13,(uint32_t)isr13,  0x08, 0x8e );
	idt_set_gate( 14,(uint32_t)isr14,  0x08, 0x8e );
	idt_set_gate( 15,(uint32_t)isr15,  0x08, 0x8e );
	idt_set_gate( 16,(uint32_t)isr16,  0x08, 0x8e );
	idt_set_gate( 17,(uint32_t)isr17,  0x08, 0x8e );
	idt_set_gate( 18,(uint32_t)isr18,  0x08, 0x8e );
	idt_set_gate( 19,(uint32_t)isr19,  0x08, 0x8e );
	idt_set_gate( 20,(uint32_t)isr20,  0x08, 0x8e );
	idt_set_gate( 21,(uint32_t)isr21,  0x08, 0x8e );
	idt_set_gate( 22,(uint32_t)isr22,  0x08, 0x8e );
	idt_set_gate( 23,(uint32_t)isr23,  0x08, 0x8e );
	idt_set_gate( 24,(uint32_t)isr24,  0x08, 0x8e );
	idt_set_gate( 25,(uint32_t)isr25,  0x08, 0x8e );
	idt_set_gate( 26,(uint32_t)isr26,  0x08, 0x8e );
	idt_set_gate( 27,(uint32_t)isr27,  0x08, 0x8e );
	idt_set_gate( 28,(uint32_t)isr28,  0x08, 0x8e );
	idt_set_gate( 29,(uint32_t)isr29,  0x08, 0x8e );
	idt_set_gate( 30,(uint32_t)isr30,  0x08, 0x8e );
	idt_set_gate( 31,(uint32_t)isr31,  0x08, 0x8e );

	idt_set_gate( 32, (uint32_t)irq0,  0x08, 0x8e );
	idt_set_gate( 33, (uint32_t)irq1,  0x08, 0x8e );
	idt_set_gate( 34, (uint32_t)irq2,  0x08, 0x8e );
	idt_set_gate( 35, (uint32_t)irq3,  0x08, 0x8e );
	idt_set_gate( 36, (uint32_t)irq4,  0x08, 0x8e );
	idt_set_gate( 37, (uint32_t)irq5,  0x08, 0x8e );
	idt_set_gate( 38, (uint32_t)irq6,  0x08, 0x8e );
	idt_set_gate( 39, (uint32_t)irq7,  0x08, 0x8e );
	idt_set_gate( 40, (uint32_t)irq8,  0x08, 0x8e );
	idt_set_gate( 41, (uint32_t)irq9,  0x08, 0x8e );
	idt_set_gate( 42, (uint32_t)irq10, 0x08, 0x8e );
	idt_set_gate( 43, (uint32_t)irq11, 0x08, 0x8e );
	idt_set_gate( 44, (uint32_t)irq12, 0x08, 0x8e );
	idt_set_gate( 45, (uint32_t)irq13, 0x08, 0x8e );
	idt_set_gate( 46, (uint32_t)irq14, 0x08, 0x8e );
	idt_set_gate( 47, (uint32_t)irq15, 0x08, 0x8e );

	idt_set_gate( 0x30,(uint32_t)isr48,  0x08, 0x8e );	 	//Dump registers
	idt_set_gate( 0x50,(uint32_t)isr80,  0x08, 0x8e | 0x60 ); 	//Syscall

	idt_flush((uint32_t)&idt_ptr );
}

static void gdt_set_gate( int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran ){
	gdt_entries[num].base_low	= ( base & 0xffff );
	gdt_entries[num].base_middle	= ( base >> 16 ) & 0xff;
	gdt_entries[num].base_high	= ( base >> 24 ) & 0xff;

	gdt_entries[num].limit_low	= (limit & 0xffff);
	gdt_entries[num].granularity	= (limit >> 16 ) & 0x0f;

	gdt_entries[num].granularity	|= gran & 0xf0;
	gdt_entries[num].access		= access;
}

static void idt_set_gate( uint8_t num, uint32_t base, uint16_t sel, uint8_t flags ){
	idt_entries[num].base_low	= base & 0xffff;
	idt_entries[num].base_high	= ( base >> 16 ) & 0xffff;

	idt_entries[num].sel		= sel;
	idt_entries[num].always0	= 0;
	//idt_entries[num].flags		= flags | 0x60 /* <-Uncomment for usermode */;
	idt_entries[num].flags		= flags;
}

static void write_tss( int32_t num, uint16_t ss0, uint32_t esp0 ){
	uint32_t base = (uint32_t)&tss_entry;
	uint32_t limit = base + sizeof( tss_entry );

	gdt_set_gate( num, base, limit, 0xe9, 0 );
	memset( &tss_entry, 0, sizeof( tss_entry ));

	tss_entry.ss0  = ss0;
	tss_entry.esp0 = esp0;

	tss_entry.cs   = 0x0b;
	tss_entry.ss   = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}
#endif
