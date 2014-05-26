#include <base/syscalls.h>
#include <base/arch/i386/isr.h>
#include <base/logger.h>
#define CHANGEME_MAX_SYSCALLS 64

static void syscall_handler( registers_t *regs );

static void *syscall_table[CHANGEME_MAX_SYSCALLS] = {
	syscall_test,
	0,
};

DEFN_SYSCALL0( test, SYSCALL_TEST );
DEFN_SYSCALL1( exit, SYSCALL_EXIT, int );
DEFN_SYSCALL2( open, SYSCALL_OPEN, char *, int );
DEFN_SYSCALL1( close, SYSCALL_CLOSE, int );
DEFN_SYSCALL3( read, SYSCALL_READ, int, void *, int );
DEFN_SYSCALL3( write, SYSCALL_WRITE, int, void *, int );

void init_syscalls( ){
	register_syscall( SYSCALL_TEST, syscall_tester );
	register_interrupt_handler( 0x50, syscall_handler );
}

int syscall_tester( ){
	kprintf( "[%s] Aw yus, syscalls be workin'\n", __func__ );

	return 0;
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

