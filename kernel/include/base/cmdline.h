#ifndef _helix_cmdline_h
#define _helix_cmdline_h
#include <base/lib/stdbool.h>

typedef struct cmdline_opt {
	char *key;
	char *value;
} cmdline_opt_t;

cmdline_opt_t *parse_command_line( char *cmdline );
char *cmdline_get( cmdline_opt_t *opts, char *key );
bool cmdline_has( cmdline_opt_t *opts, char *key );

#endif
