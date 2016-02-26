#ifndef _helix_sockets_module_h
#define _helix_sockets_module_h
#include <base/vfs/vfs.h>
#include <base/errors.h>

typedef enum {
	SOCK_DOMAIN_UNIX,
	SOCK_DOMAIN_INET,
	SOCK_DOMAIN_INET6,
	SOCK_DOMAIN_X25,
	SOCK_DOMAIN_AX25,
} sock_domain_type_t;

typedef enum {
	SOCK_TYPE_STREAM,
	SOCK_TYPE_DGRAM,
	SOCK_TYPE_SEQPACKET,
	SOCK_TYPE_RAW,
	SOCK_TYPE_RDM,
} sock_type_t;

typedef struct socket {
	sock_domain_type_t domain;
	sock_type_t        type;
	void *data;
} socket_t;

typedef struct sock_addr {
	unsigned family;
	char     data[14];
} sock_addr_t;

typedef int (*sock_domain_socket)( socket_t *sock );
typedef int (*sock_domain_bind)( socket_t *sock, sock_addr_t *addr, unsigned len );
typedef int (*sock_domain_connect)( socket_t *sock, sock_addr_t *addr, unsigned len );
typedef int (*sock_domain_listen)( socket_t *sock, unsigned backlog );
typedef int (*sock_domain_close)( socket_t *sock );
typedef int (*sock_domain_unlink)( socket_t *sock );
typedef file_event_t (*sock_domain_poll)( socket_t *sock, file_event_t mask );

typedef struct sock_domain {
	sock_domain_socket  socket;
	sock_domain_bind    bind;
	sock_domain_connect connect;
	sock_domain_listen  listen;
	sock_domain_close   close;
	sock_domain_unlink  unlink;
	sock_domain_poll    poll;
} sock_domain_t;

int sock_register_domain( sock_domain_type_t dtype, sock_domain_t *domain );
int sock_unregister_domain( sock_domain_type_t dtype );

#endif
