#include <stdio.h>
#include <dalibc/syscalls.h>

typedef struct dirent {
	unsigned inode;
	unsigned offset;
	unsigned length;
	unsigned type;

	char     name[256];
} dirent_t;

int syscall_readdir( int fd, dirent_t *dirent, int entry );

int main( int argc, char *argv[], char *envp[] ){
	dirent_t dir;
	int arg, i, fd;

	if ( argc > 1 ){
		for ( arg = 1; arg < argc; arg++ ){
			fd = open( argv[arg], 0 );

			if ( fd >= 0 ){
				for ( i = 0; syscall_readdir( fd, &dir, i ) > 0; i++ ){
					puts( dir.name );
				}

			} else {
				printf( "could not open \"%s\"\n", argv[arg] );
				break;
			}
		}

	} else {
		fd = open( ".", 0 );

		if ( fd >= 0 ){
			for ( i = 0; syscall_readdir( fd, &dir, i ) > 0; i++ ){
				puts( dir.name );
			}

		} else {
			printf( "could not open \".\"\n" );
		}
	}

	return 0;
}
