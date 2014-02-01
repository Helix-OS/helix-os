#ifndef _helix_tasking_h
#define _helix_tasking_h

typedef struct task {
	unsigned long 	eip,
			esp,
			ebp;

	unsigned long 	pid;
	unsigned long 	sleep;
} task_t;

#endif
