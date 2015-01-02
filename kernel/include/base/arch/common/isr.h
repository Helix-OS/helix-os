#ifndef _helix_isr_h
#define _helix_isr_h

#include <base/stdint.h>
#include <base/kstd.h>

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

typedef struct registers {
	uint32_t	ds;
	uint32_t	edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t	int_no, err_code;
	uint32_t	eip, cs, eflags, useresp, ss;
} registers_t;

typedef void (*isr_t)(registers_t *);
void register_interrupt_handler( uint8_t, isr_t );
void unregister_interrupt_handler( uint8_t );
void dump_registers( registers_t *regs );

void zero_division_fault( registers_t *regs );
void invalid_op_fault( registers_t *regs );
void gen_protect_fault( registers_t *regs );
void double_fault( registers_t *regs );

#endif
