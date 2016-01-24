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

int syscall_sysinfo( unsigned type, unsigned key, void *buf );

int main( int argc, char *argv[], char *envp[] ){
	kinfo_t infobuf;

	syscall_sysinfo( SYSINFO_KERNEL, 0, &infobuf );

	printf(
		"kernel version: %u\n"
		"     page size: %u bytes\n"
		"    free pages: %u\n",
		infobuf.version, infobuf.page_size, infobuf.free_pages );

	return 0;
}
