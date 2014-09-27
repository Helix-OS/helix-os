#include <dalibc/syscalls.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *getstuff( char *s, int lim ){
	char *ret = s;
	int i, c;

	c = getchar( );
	for ( i = 0; c != '\n' && i < lim; i++ ){
		putchar( c );
		s[i] = c;
		c = getchar( );
	}

	s[i] = 0;

	return ret;
}

int main( int argc, char *argv[], char *envp[] ){
	char *username = malloc( 32 );
	char *password = malloc( 32 );
	unsigned pid;
	int status;

	while ( 1 ){
		printf( "username: " );
		username = getstuff( username, 32 );
		putchar( '\n' );

		printf( "password: " );
		password = fgets( password, 32, stdin );
		*strchr( password, '\n' ) = 0;
		putchar( '\n' );

		if (( strcmp( username, "admin" ) == 0 ) && ( strcmp( password, "password" ) == 0 )){
			printf( "You have been successfully authenticated.\n" );

			pid = _spawn( "/test/userroot/bin/sh", NULL, NULL );
			syscall_waitpid( pid, &status, 0 );

		} else { 
			int i;
			printf( "Could not authenticate.\n" );
			printf( "username: %s, password: ", username );
			for ( i = 0; password[i]; i++ ){
				putchar( '*' );
			}
			putchar( '\n' );
		}
	}

	return 0;
}
