#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

typedef struct mtable_header {
	uint32_t entries;
	uint32_t entry_size;
	uint32_t table_offset;
	uint32_t magic;
} mhead_t;

typedef struct mod_table {
	uint32_t offset;
	uint32_t len;
	uint32_t checksum;
	uint32_t magic;
} mtable_t;

int main( int argc, char *argv[] ){
	FILE	*out,
		*temp;
	char	*out_name,
		*buf;
	struct stat sb;
	int i, read, total;
	uint32_t offset;

	mhead_t *header;
	mtable_t *table;
	
	if ( argc < 3 ){
		printf( "Usage: %s [output file] [input file]\n", argv[0] );
		return 0;
	}

	out_name = argv[1];

	if (( out = fopen( out_name, "w" )) == NULL ){
		perror( "Could not open output file\n" );
		return -1;
	}

	header = calloc( 1, sizeof( mhead_t ));
	table = calloc( argc - 2, sizeof( mtable_t ));

	header->entries = (unsigned)( argc - 2 );
	header->entry_size = sizeof( mtable_t );
	header->table_offset = sizeof( mhead_t );
	header->magic = 0xbadc0de;

	offset = sizeof( mtable_t ) * header->entries + header->table_offset;
	printf( "Entries: %d, entry size: %d, offset: %d, magic: 0x%x\n",
		header->entries, header->entry_size, header->table_offset, header->magic );

	fwrite( header, sizeof( mhead_t ), 1, out );

	for ( i = 2; i < argc; i++ ){
		if ( stat( argv[i], &sb ) < 0 ){
			perror( argv[i] );
			return -2;
		}

		table[i-2].offset = offset;
		table[i-2].len = sb.st_size;
		table[i-2].magic = 0xbadc0de;
		fwrite( table + i - 2, sizeof( mtable_t ), 1, out );
		printf( "Wrote table %d, len = %d, offset = %d...\n", i - 2, table[i-2].len, table[i-2].offset );

		offset += sb.st_size;
	}

	free( header );
	free( table );

	buf = malloc( 128 );
	for ( i = 2; i < argc; i++ ){
		if (( temp = fopen( argv[i], "r" )) == NULL ){
			perror( argv[i] );
			return -3;
		}

		total = 0;
		while ( 1 ){
			read = fread( buf, 1, 128, temp );
			if ( read <= 0 )
				break;

			total += fwrite( buf, 1, read, out );
			printf( "Copied %d bytes of \"%s\" to output...\r", total, argv[i] );
		}
		printf( "\n" );

		fclose( temp );
	}

	free( buf );
	fclose( out );

	return 0;
}
