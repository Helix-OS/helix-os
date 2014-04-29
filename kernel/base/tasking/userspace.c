#include <base/tasking/task.h>

void switch_to_usermode( void ){
	asm volatile(	" 			\
			cli;			\
			mov $0x23, %ax; 	\
			mov %ax, %ds;		\
			mov %ax, %es;		\
			mov %ax, %fs;		\
			mov %ax, %gs;		\
						\
			mov %esp, %eax;		\
			pushl $0x23;		\
			pushl %eax;		\
			pushf;			\
			popl %eax;		\
			orl $0x200, %eax;	\
			pushl %eax;		\
			pushl $0x1b;		\
			pushl $1f;		\
			iret;			\
			1:			\
			"
	);
}

void switch_to_usermode_jmp( void *addr ){
	asm volatile(	" 			\
			cli;			\
			mov $0x23, %%ax; 	\
			mov %%ax, %%ds;		\
			mov %%ax, %%es;		\
			mov %%ax, %%fs;		\
			mov %%ax, %%gs;		\
						\
			mov %%esp, %%eax;	\
			pushl $0x23;		\
			pushl %%eax;		\
			pushf;			\
			popl %%eax;		\
			orl $0x200, %%eax;	\
			pushl %%eax;		\
			pushl $0x1b;		\
			pushl %0;		\
			iret;			\
			" 
	:: "m"( addr ));
}
