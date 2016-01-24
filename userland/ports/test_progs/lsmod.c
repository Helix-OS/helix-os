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
	unsigned tid;
	unsigned parent;
	unsigned pid;
	unsigned state;
} proc_info_t;

typedef struct mod_info {
	char name[32];
	unsigned long address;
	unsigned long npages;
} mod_info_t;

int syscall_sysinfo( unsigned type, unsigned key, void *buf );

unsigned numlen( unsigned n ){
	unsigned ret = 1;

	for ( ; n - (n % 10) > 0; ret++, n /= 10 );

	return ret;
}

int main( int argc, char *argv[], char *envp[] ){
	unsigned i;
	mod_info_t modbuf;

	printf( "     name | size | address\n" );
	for ( i = 0; syscall_sysinfo( SYSINFO_MODULES, i, &modbuf ) > 0; i++ ){
		unsigned k;
		for ( k = 9 - strlen( modbuf.name ); k; k-- ){
			putchar( ' ' );
		}

		printf( "%s", modbuf.name );
		printf( " |    %d", modbuf.npages );
		printf( " | 0x%x", modbuf.address );
		putchar( '\n' );
	}

	return 0;
}
