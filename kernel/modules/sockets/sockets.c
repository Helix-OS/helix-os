#include <sockets/sockets.h>
#include <base/datastructs/pipe.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>
#include <base/tasking/task.h>
#include <base/vfs/vfs.h>
#include <base/syscalls.h>
#include <base/logger.h>

char *depends[] = { "base", 0 };
char *provides = "sockets";

// socket syscalls
static int sock_socket( int domain, int type, int protocol );
static int sock_socketpair( int domain, int type, int protocol, int *sv );
static int sock_bind( int fd, const sock_addr_t *addr, unsigned len );
static int sock_listen( int fd, int backlog );
static int sock_connect( int fd, sock_addr_t *addr, unsigned len );

// interface to VFS
//static int socket_vfs_open( struct file_node *, char *, int );
static int socket_vfs_close( struct file_node *node, int flags );
static int socket_vfs_write( struct file_node *node, void *buf, size_t length, size_t offset );
static int socket_vfs_read( struct file_node *node, void *buf, size_t length, size_t offset );
static file_event_t socket_vfs_poll( struct file_node *node, file_event_t mask );

static file_funcs_t socket_vfs_funcs = {
	.read  = socket_vfs_read,
	.write = socket_vfs_write,
	.close = socket_vfs_close,
	.poll  = socket_vfs_poll,
};

static file_system_t socket_vfs_fs = {
	.functions = &socket_vfs_funcs,
};

static int sock_socket( int domain, int type, int protocol ){
	return 0;
}

static int sock_socketpair( int domain, int type, int protocol, int *sv ){
	return 0;
}

static int sock_bind( int fd, const sock_addr_t *addr, unsigned len ){
	return 0;
}

static int sock_listen( int fd, int backlog ){
	return 0;
}

static int sock_connect( int fd, sock_addr_t *addr, unsigned len ){
	return 0;
}

static int socket_vfs_close( struct file_node *node, int flags ){
	return 0;
}

static int socket_vfs_write( struct file_node *node, void *buf, size_t length, size_t offset ){
	return 0;
}

static int socket_vfs_read( struct file_node *node, void *buf, size_t length, size_t offset ){
	return 0;
}

static file_event_t socket_vfs_poll( struct file_node *node, file_event_t mask ){
	return 0;
}

int init( ){
	kprintf( "[%s] Hello world!\n", provides );
	kprintf( "[%s] This is module \"%s\", and I'm in yo kernelz\n",
			provides, provides );

	register_syscall( SYSCALL_SOCKET, sock_socket );
	register_syscall( SYSCALL_SOCKETPAIR, sock_socketpair );
	register_syscall( SYSCALL_BIND, sock_bind );
	register_syscall( SYSCALL_LISTEN, sock_listen );
	register_syscall( SYSCALL_CONNECT, sock_connect );

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
