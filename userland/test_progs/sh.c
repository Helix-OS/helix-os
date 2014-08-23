#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main( int argc, char *argv[], char *envp[] ){
	char *buf = malloc( 128 );
	int i, c;
	int running = 1;

	while ( running ){
		printf( "$ " );

		c = getchar( );
		for ( i = 0; i < 128 && c != '\n'; i++ ){
			putchar( c );
			buf[i] = c;
			c = getchar( );
		}
		buf[i] = 0;
		putchar( '\n' );

		if ( strlen( buf )){
			printf( "Got \"%s\"\n", buf );

			if (( strcmp( buf, "exit" )) == 0 )
				running = 0;
		}
	}

	return 0;
}
