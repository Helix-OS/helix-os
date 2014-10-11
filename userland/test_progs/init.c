#include <dalibc/syscalls.h>
#include <stdio.h>
#include <stdlib.h>

static char *init_commands[] = {
	"/bin/logind",
	NULL,
};

int main( int argc, char *argv[], char *envp[] ){
	int i;
	char *asdf = malloc( 32 );
	char *userroot = "/test/userroot";

	puts( "Hello, world!" );
	syscall_chroot( userroot );
	printf( "Changed root to %s\n", userroot );

	for ( i = 0; init_commands[i]; i++ ){
		printf( "starting %s... ", init_commands[i] );
		_spawn( init_commands[i], NULL, NULL );
		printf( "done\n" );
	}

	return 0;
}
