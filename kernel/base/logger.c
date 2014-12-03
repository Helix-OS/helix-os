#include <base/logger.h>
#include <base/stdio.h>
#include <base/string.h>
#include <base/serial.h>

char *logputs( char *str ){
	int i;
	for ( i = 0; str[i]; i++ );

	write_serial( str, i );

	return str;
}

void print_num( unsigned input ){
	unsigned i, a;
	char	buf[32];

	if ( !input ){
		buf[0] = '0';
		write_serial( buf, 1 );
		return;
	}

	for ( i = 0, a = input; a; a = a / 10 )
		buf[i++] = a % 10 + '0';

	while( i-- )
		write_serial( &buf[i], 1 );
}

int atoi( char *n ){
	int ret = 0;
	char *s = n;

	for ( ; *s; s++ ){
		ret *= 10;
		ret += *s - '0';
	}

	return ret;
}

void print_hex( unsigned input ){
	unsigned i, a;
	char	buf[32],
		*hextab = "0123456789abcdef";

	if ( !input ){
		buf[0] = '0';
		write_serial( buf, 1 );
		return;
	}

	for ( i = 0, a = input; a; a = a / 16 )
		buf[i++] = hextab[ a % 16 ];

	while( i-- )
		write_serial( &buf[i], 1 );
}

int kvprintf( char *format, va_list args ){
	int		slen = strlen( format ),
			i = 0,
			signed_int;
	unsigned	unsigned_int;
	char 		buf,
			*str;

	for ( i = 0; i < slen; i++ ){
		if ( format[i] == '%' ){
			switch( format[++i] ){
				case '%':
					buf = '%';
					write_serial( &buf, 1 );
					break;
				case 'c':
					buf = va_arg( args, int );
					write_serial( &buf, 1 );
					break;
				case 's':
					str = va_arg( args, char * );
					logputs( str );
					break;
				case 'd':
					signed_int = va_arg( args, int );
					print_num( signed_int );
					break;
				case 'u':
					unsigned_int = va_arg( args, unsigned );
					print_num( unsigned_int );
					break;
				case 'x':
					unsigned_int = va_arg( args, unsigned );
					print_hex( unsigned_int );
					break;
			}
		} else {
			write_serial( format + i, 1 );
		}
	}

	return i;
}

int kprintf( char *format, ... ){
	int i;
	va_list args;
	va_start( args, format );

	i = kvprintf( format, args );

	va_end( args );
	return i;
}
