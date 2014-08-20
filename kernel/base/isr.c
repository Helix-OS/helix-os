#ifndef _kernel_isr_c
#define _kernel_isr_c

#include <base/arch/i386/isr.h>
#include <base/logger.h>
#include <base/stdio.h>

isr_t interrupt_handlers[256];
unsigned long 	isr_error_count = 0,
		error_threshold = 10;

void isr_handler( registers_t regs ){
	if ( interrupt_handlers[regs.int_no] != 0 ){
		isr_t handler = interrupt_handlers[regs.int_no];
		handler( &regs );
	} else {
		kprintf( "Unhandled interrupt: 0x%x : 0x%x\n", regs.int_no, regs.err_code );
		asm volatile( "cli; hlt" );
	}
}

void irq_handler( registers_t regs ){
	if ( regs.int_no >= 40 ){
		outb( 0xa0, 0x20 );
	}
	outb( 0x20, 0x20 );
	
	if ( interrupt_handlers[regs.int_no] != 0 ){
		isr_t handler = interrupt_handlers[regs.int_no];
		handler( &regs );
	}
}

void register_interrupt_handler( uint8_t n, isr_t handler ){
	interrupt_handlers[n] = handler;
}

void unregister_interrupt_handler( uint8_t n ){
	interrupt_handlers[n] = 0;
}

void dump_registers( registers_t *regs ){
	int cr0, cr2, cr3;

	kprintf("eax: 0x%x\tebx: 0x%x\tecx: 0x%x\tedx: 0x%x\n"
		"edi: 0x%x\tesi: 0x%x\tesp: 0x%x\tebp: 0x%x\t\n"
		"eip: 0x%x\tuesp:0x%x\t cs: 0x%x\t ss: 0x%x\n"
		" ds: 0x%x\tint: 0x%x\terr: 0x%x\tefl: 0x%s\n",
		regs->eax, regs->ebx, regs->ecx, regs->edx, 
		regs->edi, regs->esi, regs->esp, regs->ebp, 
		regs->eip, regs->useresp,  regs->cs,  regs->ss,
		regs->ds, regs->int_no, regs->err_code, regs->eflags 
	);

	asm volatile( "mov %%cr0, %0" : "=r"(cr0));
	//asm volatile( "mov %%cr1, %0" : "=r"(cr1));
	asm volatile( "mov %%cr2, %0" : "=r"(cr2));
	asm volatile( "mov %%cr3, %0" : "=r"(cr3));

	kprintf("cr0: 0x%x\tcr2: 0x%x\tcr3: 0x%x\n",
		cr0, cr2, cr3 );
}

void zero_division_fault( registers_t *regs ){
	dump_registers( regs );
	panic( "Zero division fault\n" );
}

void invalid_op_fault( registers_t *regs ){
	dump_registers( regs );
	panic( "Invalid opcode\n" );
}

void gen_protect_fault( registers_t *regs ){
	dump_registers( regs );
	panic( "General protection fault\n" );
}

void double_fault( registers_t *regs ){
	logputs( "Double fault\n" );
	dump_registers( regs );

	while( 1 ) asm ( "hlt" );
}

#endif
