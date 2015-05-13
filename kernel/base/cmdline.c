#include <base/cmdline.h>
#include <base/string.h>

// FIXME: Giving a longer (> 4 or 5 chars) command line option causes a page fult
//        When the user process starts, in the amalloc() function
//        So most definitely heap corruption, but where?
// TODO:  Ok seriously, rewrite the kernel malloc()
cmdline_opt_t *parse_command_line( char *cmdline ){
	cmdline_opt_t *ret = NULL;
	cmdline_opt_t *opts;

	if ( cmdline ){
		unsigned i, k;
		unsigned entries;  // the total number of command line options
		unsigned pos;      // the start of the last value string
		unsigned len;
		bool has_val = false;
		bool done = false;

		kprintf( "[%s] Have command line \"%s\"\n", __func__, cmdline );

		for ( i = entries = 0; cmdline[i]; i++ ){
			if ( cmdline[i] == ' ' || (cmdline[i] != ' ' && !entries))
				entries++;
		}

		ret = opts = knew( sizeof( cmdline_opt_t[entries + 1] ));
		kprintf( "[%s] Have %d command line options\n", __func__, entries );

		for ( k = pos = i = 0; !done; i++ ){
			switch( cmdline[i] ){
				case '=':
					len = i - pos;

					opts[k].key = knew( sizeof( char[len + 1] ));
					strncpy( opts[k].key, cmdline + pos, len );
					opts[k].key[len] = 0;
					kprintf( "option \"%s\" = ", opts[k].key );

					has_val = true;
					pos = i + 1;
					break;

				case '\0':
					done = true;

				case ' ':
					if ( !has_val ){
						len = i - pos;
						opts[k].key = knew( sizeof( char[len + 1] ));
						strncpy( opts[k].key, cmdline + pos, len );
						opts[k].key[len] = 0;
						opts[k].value = strdup( "true" );
						kprintf( "option \"%s\" = ", opts[k].key );

					} else {
						len = i - pos;
						opts[k].value = knew( sizeof( char[len + 1] ));
						strncpy( opts[k].value, cmdline + pos, len );
						opts[k].value[len] = 0;
					}

					kprintf( "\"%s\"\n", opts[k].value );

					has_val = false;
					k++;
					pos = i + 1;
					break;

				default:
					break;
			}
		}
	}

	return ret;
}

char *cmdline_get( cmdline_opt_t *opts, char *key ){
	char *ret = NULL;
	unsigned i;
	
	if ( opts ){
		for ( i = 0; opts[i].key; i++ ){
			if ( strcmp( key, opts[i].key ) == 0 ){
				ret = opts[i].value;
				break;
			}
		}
	}

	return ret;
}

bool cmdline_has( cmdline_opt_t *opts, char *key ){
	bool ret = false;
	unsigned i;
	
	if ( opts ){
		for ( i = 0; opts[i].key; i++ ){
			if ( strcmp( key, opts[i].key ) == 0 ){
				ret = true;
				break;
			}
		}
	}

	return ret;
}
