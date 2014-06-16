#include <dalibc/syscalls.h>

int main( int argc, char *argv[], char *envp[] ){
	int keyboard = open( "/test/devices/keyboard", 1 );
	int video = open( "/test/devices/console", 2 );
	int i;
	char asdf[32];

	write( video, "Hello, world!\n", 15 );

	while ( 1 ){
		i = read( keyboard, asdf, 32 );
		write( video, asdf, i );
	}

	return 0;
}
