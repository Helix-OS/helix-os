#ifndef _helix_kernel_initrd
#define _helix_kernel_initrd
#include <base/errors.h>
#include <base/datastructs/dlist.h>

typedef struct tar_header {
	char filename[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char typeflag[1];
	char linkname[100];
	char pad[255];
} tar_header_t;

typedef struct initrd {
	tar_header_t *firsthead;
	dlist_container_t *headers;
} initrd_t;

initrd_t *init_initrd( tar_header_t *header );
tar_header_t *initrd_get( initrd_t *rd, int index );
tar_header_t *initrd_get_file( initrd_t *rd, char *name );
unsigned initrd_get_size( tar_header_t *header );

#endif
