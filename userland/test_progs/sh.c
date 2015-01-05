#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dalibc/syscalls.h>

int running = 1;

char *getline( char *buf, unsigned len ){
	unsigned i;
	char c;

	c = getchar( );
	for ( i = 0; i < len && c != '\n'; ){

		if ( c == '\b' && i > 0 ){
			putchar( '\b' );
			buf[i] = 0;
			i--;

		} else if ( c != '\b' ){
			buf[i] = c;
			putchar( c );
			i++;
		}

		c = getchar( );
	}

	buf[i] = 0;

	return buf;
}

char **split_str( char *s, char split ){
	unsigned i, nchars, j;
	char **ret = NULL;

	for ( nchars = i = 0; s[i]; i++ ){
		if ( s[i] == split )
			nchars++;
	}

	if ( nchars ){
		ret = malloc( sizeof( char *[nchars + 2] ));

		for ( j = 0, i = 0; s[i]; i++ ){
			if ( i == 0 ){
				ret[j] = s;
				j++;

			} else if ( s[i] == split ){
				s[i] = 0;
				ret[j] = s + i + 1;
				j++;
			}
		}

		ret[j] = NULL;

	} else {
		ret = malloc( sizeof( char *[2] ));
		ret[0] = s;
		ret[1] = NULL;
	}

	return ret;
}

int exec_cmd( char *args[], char *env[] ){
	int pid;
	int ret = 0;

	if ( args ){
		if ( strlen( args[0] ) > 0 ){
			pid = _spawn( args[0], args, NULL );

			if ( pid > 0 ){
				syscall_waitpid( pid, &ret, 0 );

			} else {
				puts( "Command not found." );
			}
		}

		puts( "Got here." );
		printf( "\"%s\"\n", args[0] );
	}

	return ret;
}

int main( int argc, char *argv[], char *envp[] ){
	char *buf = malloc( 128 );
	int ret;

	while ( running ){
		printf( "$ " );
		getline( buf, 128 );
		putchar( '\n' );
		ret = exec_cmd( split_str( buf, ' ' ), NULL );
	}

	return ret;
}
