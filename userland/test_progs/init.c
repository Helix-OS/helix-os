#include <dalibc/syscalls.h>
#include <stdio.h>
#include <stdlib.h>

static char *init_commands[] = {
	"/test/userroot/bin/logind",
	NULL,
};

int main( int argc, char *argv[], char *envp[] ){
	int i;
	char *asdf = malloc( 32 );

	puts( "Hello, world!" );

	for ( i = 0; init_commands[i]; i++ ){
		printf( "starting %s... ", init_commands[i] );
		_spawn( init_commands[i], NULL, NULL );
		printf( "done\n" );
	}

	return 0;
}
