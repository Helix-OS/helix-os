#include <base/syscalls.h>
#include <arch/isr.h>
#include <base/logger.h>
#define CHANGEME_MAX_SYSCALLS 64

static void syscall_handler( registers_t *regs );

static void *syscall_table[CHANGEME_MAX_SYSCALLS] = {
	syscall_test,
	0,
};

void arch_init_syscalls( ){
	register_interrupt_handler( 0x50, syscall_handler );
}

void register_syscall( syscall_t n, void *call ){
	if ( n < CHANGEME_MAX_SYSCALLS )
		syscall_table[n] = call;
}

static void syscall_handler( registers_t *regs ){
	if ( regs->eax >= CHANGEME_MAX_SYSCALLS )
		return;

	void *location = syscall_table[regs->eax];
	if ( !location )
		// Syscall not registered
		return;

    //kprintf( "[%s] Have syscall %d at 0x%x\n", __func__, regs->eax, location );

	int ret;
	asm volatile( "	\
	push %1;	\
	push %2;	\
	push %3;	\
	push %4;	\
	push %5;	\
	call *%6;	\
	pop %%ebx;	\
	pop %%ebx;	\
	pop %%ebx;	\
	pop %%ebx;	\
	pop %%ebx;	\
	" : "=a"(ret) : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx), "r"(regs->ecx), "r"(regs->ebx), "r"(location));
	regs->eax = ret;
}

