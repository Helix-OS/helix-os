#include <base/syscalls.h>
#include <base/arch/i386/isr.h>
#include <base/logger.h>
#define CHANGEME_MAX_SYSCALLS 64

static void syscall_handler( registers_t *regs );

static void *syscall_table[CHANGEME_MAX_SYSCALLS] = {
	syscall_test,
	0,
};

DEFN_SYSCALL0( syscall_test, 0 );

void init_syscalls( ){
	register_interrupt_handler( 0x50, syscall_handler );
}

int syscall_test( ){
	kprintf( "[%s] Aw yus, syscalls be workin'\n", __func__ );

	return 0;
}

static void syscall_handler( registers_t *regs ){
	if ( regs->eax >= CHANGEME_MAX_SYSCALLS )
		return;

	void *location = syscall_table[regs->eax];
	if ( !location )
		// Syscall not registered
		return;

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

