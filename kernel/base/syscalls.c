#include <base/syscalls.h>
#include <base/logger.h>
#include <base/sysinfo.h>

DEFN_SYSCALL0( test, SYSCALL_TEST );
DEFN_SYSCALL1( exit, SYSCALL_EXIT, int );
DEFN_SYSCALL2( open, SYSCALL_OPEN, char *, int );
DEFN_SYSCALL1( close, SYSCALL_CLOSE, int );
DEFN_SYSCALL3( read, SYSCALL_READ, int, void *, int );
DEFN_SYSCALL3( write, SYSCALL_WRITE, int, void *, int );
DEFN_SYSCALL4( spawn, SYSCALL_SPAWN, int, char **, char **, int * );
DEFN_SYSCALL3( readdir, SYSCALL_READDIR, int, void *, int );
DEFN_SYSCALL3( waitpid, SYSCALL_WAITPID, unsigned, int *, int );
DEFN_SYSCALL1( sbrk, SYSCALL_SBRK, int );
DEFN_SYSCALL1( chroot, SYSCALL_CHROOT,  const char * );
DEFN_SYSCALL1( chdir, SYSCALL_CHDIR, const char * );
DEFN_SYSCALL3( sysinfo, SYSCALL_SYSINFO, unsigned, unsigned, void * );
DEFN_SYSCALL3( lseek, SYSCALL_LSEEK, int, long, int );
DEFN_SYSCALL1( pipe, SYSCALL_PIPE, int * );
DEFN_SYSCALL3( fcntl, SYSCALL_FCNTL, int, int, int );
//DEFN_SYSCALL2( fstat, SYSCALL_FSTAT, int, void * );

void init_syscalls( ){
	arch_init_syscalls( );
	register_syscall( SYSCALL_TEST, syscall_tester );
	register_syscall( SYSCALL_SYSINFO, sysinfo );
}

int syscall_tester( ){
	kprintf( "[%s] Aw yus, syscalls be workin'\n", __func__ );

	return 0;
}
