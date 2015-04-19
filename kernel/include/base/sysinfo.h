#ifndef _helix_sysinfo_h
#define _helix_sysinfo_h

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

int sysinfo( unsigned type, unsigned key, void *buf );
int get_kernel_info( unsigned key, kinfo_t *buf );
int get_process_info( unsigned key, proc_info_t *buf );

#endif
