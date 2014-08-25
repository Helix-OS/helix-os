#include <stdio.h>

int main( int argc, char *argv[], char *envp[] ){
	FILE *fp;
	int arg, i, fd;
	char buf[512];

	for ( arg = 1; arg < argc; arg++ ){
		fp = fopen( argv[arg], "r" );

		if ( fp ){
			i = 1;

			while ( i > 0 ){
				i = fread( buf, 512, 1, fp );
				fwrite( buf, i, 1, stdout );
			}

		} else {
			printf( "could not open \"%s\"\n", argv[arg] );
			break;
		}
	}

	return 0;
}
