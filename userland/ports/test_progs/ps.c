#include <dalibc/syscalls.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	SYSINFO_NONE,
	SYSINFO_KERNEL,
	SYSINFO_PROCESS,
	SYSINFO_PROC_ID,
	SYSINFO_MODULES,
};

typedef struct kinfo {
	unsigned version;

	// memory info
	unsigned page_size;
	unsigned free_pages;
} kinfo_t;

typedef struct proc_info {
	unsigned pid;
	unsigned state;
} proc_info_t;

enum {
	TASK_STATE_RUNNING,
	TASK_STATE_SLEEPING,
	TASK_STATE_WAITING,
	TASK_STATE_STOPPED,
	TASK_STATE_ENDED,
};

int syscall_sysinfo( unsigned type, unsigned key, void *buf );

static char *pstates[] = {
	"running",
	"sleeping",
	"waiting",
	"stopped",
	"ended",
	NULL
};

unsigned numlen( unsigned n ){
	unsigned ret = 1;

	for ( ; n - (n % 10) > 0; ret++, n /= 10 );

	return ret;
}

int main( int argc, char *argv[], char *envp[] ){
	unsigned i;
	proc_info_t procbuf;

	printf( "PID\t| state\n" );
	for ( i = 0; syscall_sysinfo( SYSINFO_PROCESS, i, &procbuf ) > 0; i++ ){
		unsigned k;
		for ( k = 3 - numlen( procbuf.pid ); k; k-- ){
			putchar( ' ' );
		}

		printf( "%d", procbuf.pid );

		printf( " | %s\n", pstates[procbuf.state] );
	}

	return 0;
}
