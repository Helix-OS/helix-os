#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dalibc/syscalls.h>

int async_getline( char *buf, unsigned len, int *state ){
	unsigned i;
	char c;
	int ret = 0;

	c = getchar( );

	for ( i = *state; c > 0 && i < (len - 1) && c != '\n'; ){
		if ( c == '\b' && i > 0 ){
			putchar( '\b' );
			putchar( '\b' );
			putchar( '_' );
			buf[i] = 0;
			i--;

		} else if ( c != '\b' ){
			buf[i] = c;
			if ( i > 0 ){
				putchar( '\b' );
			}
			putchar( c );
			putchar( '_' );
			i++;
		}

		c = getchar( );
		*state = i;
	}

	if ( c == '\n' || i == (len - 1)){
		putchar('\b');
		putchar('\n');
		ret = 1;
		buf[i] = '\n';
		buf[i+1] = 0;
		*state = 0;
	}

	return ret;
}

int main( int argc, char *argv[] ){
	int p_to[2];
	int p_from[2];

	pipe( p_to );
	pipe( p_from );
	char *buf = malloc( 256 );

	int p_fds[3] = { p_to[0], p_from[1], p_from[1] };
	int pid = _spawn( "/bin/logind", NULL, NULL, p_fds );

	if ( pid < 0 ){
		printf( "_spawn(): couldn't run initial command, %d\n", pid );
		exit( 1 );
	}

	close( p_to[0] );
	close( p_from[1] );

	int i = read( p_from[0], buf, 256 );
	fwrite( buf, 1, i, stdout );

	fcntl( 0, F_SETFL, O_NONBLOCK );
	fcntl( p_to[1], F_SETFL, O_NONBLOCK );
	fcntl( p_from[0], F_SETFL, O_NONBLOCK );

	int state;
	int has_line;

	struct pollfd polling[2];
	polling[0].fd = 0;
	polling[0].events = POLLIN;

	polling[1].fd = p_from[0];
	polling[1].events = POLLIN;

	while ( 1 ) {
		int foo = poll( polling, 2, -1 );

		if ( polling[0].revents & POLLIN ){
			has_line = async_getline( buf, 256, &state );

			if ( has_line ){
				write( p_to[1], buf, strlen( buf ));
			}
		}

		if ( polling[1].revents & POLLIN ){
			int k = read( p_from[0], buf, 256 );
			if ( k > 0 ){
				fwrite( buf, 1, k, stdout );
			}
		}

		polling[0].revents = polling[1].revents = 0;
	}

	return 0;
}
