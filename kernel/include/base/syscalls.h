#ifndef _helix_syscall_h
#define _helix_syscall_h

#define DECL_SYSCALL0(fn) int syscall_##fn();
#define DECL_SYSCALL1(fn, p1) int syscall_##fn(p1);
#define DECL_SYSCALL2(fn, p1, p2) int syscall_##fn(p1,p2);
#define DECL_SYSCALL3(fn, p1, p2, p3) int syscall_##fn(p1,p2,p3);
#define DECL_SYSCALL4(fn, p1, p2, p3, p4) int syscall_$$fn(p1,p2,p3,p4);
#define DECL_SYSCALL5(fn, p1, p2, p3, p4, p5) int syscall_$$fn(p1,p2,p3,p4,p5);

#define DEFN_SYSCALL0(fn, num) \
int syscall_##fn(){ \
	int a;\
	asm volatile( "int $0x50" : "=a" (a) : "0" (num));\
	return a;\
}

#define DEFN_SYSCALL1(fn, num, P1) \
int syscall_##fn(P1 p1){ \
	int a; \
	asm volatile( "int $0x50" : "=a" (a) : "0" (num), "b"((int)p1)); \
	return a; \
}

#define DEFN_SYSCALL2(fn, num, P1, P2) \
int syscall_##fn(P1 p1, P2 p2){ \
	int a; \
	asm volatile( "int $0x50" : "=a" (a) : "0" (num), "b"((int)p1), "c"((int)p2)); \
	return a; \
}

#define DEFN_SYSCALL3(fn, num, P1, P2, P3) \
int syscall_##fn(P1 p1, P2 p2, P3 p3){ \
	int a; \
	asm volatile( "int $0x50" : "=a" (a) : "0" (num), "b"((int)p1), "c"((int)p2), "d"((int)p3)); \
	return a; \
}

#define DEFN_SYSCALL4(fn, num, P1, P2, P3, P4) \
int syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4){ \
	int a; \
	asm volatile( "int $0x50" : "=a" (a) : "0" (num), "b"((int)p1), "c"((int)p2), "d"((int)p3), "S"((int)p4));\
	return a; \
}

typedef enum {
	SYSCALL_TEST,
	SYSCALL_EXIT,
	SYSCALL_OPEN,
	SYSCALL_CLOSE,
	SYSCALL_READ,
	SYSCALL_WRITE,
	SYSCALL_SPAWN,
	SYSCALL_READDIR,
} syscall_t;

DECL_SYSCALL0( test );
DECL_SYSCALL1( exit, int );
DECL_SYSCALL2( open, char *, int );
DECL_SYSCALL1( close, int );
DECL_SYSCALL3( read, int, void *, int );
DECL_SYSCALL3( write, int, void *, int );
DECL_SYSCALL4( spawn, int, char **, char **, int );
DECL_SYSCALL3( readdir, int, void *, int );

int syscall_tester( );
void init_syscalls( );
void register_syscall( syscall_t n, void *call );

#endif
