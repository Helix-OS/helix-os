#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dalibc/syscalls.h>

typedef enum type {
	TYPE_NULL,
	TYPE_COMMAND,
	TYPE_QUOTED,
	TYPE_VARIABLE,
	TYPE_NEWLINE,
} type_t;

typedef struct sh_token {
	char *str;
	struct sh_token *next;
	type_t type;
} sh_token_t;

typedef struct parsed_command {
	char **args;
	unsigned num_args;
} parsed_command_t;

sh_token_t *read_token( FILE *stream ){
	sh_token_t *ret = NULL;
	unsigned alloced = 32; // TODO: dynamically reallocate
	char *buf = malloc( alloced );
	int found = 0;
	int c;
	int i = 0;

	while ( !found && i < alloced ){
		c = fgetc( stream );

		if ( c > ' ' && c < 0x7f ){
			buf[i] = c;
			putchar( c );

		} else if ( c == '\b' ){
			if ( i > 0 ){
				buf[i] = 0;
				i--;
				putchar( c );
			}

		} else if ( c == ' ' ){
			buf[i] = 0;
			found = 1;

			ret = malloc( sizeof( sh_token_t ));
			ret->str = buf;
			ret->type = TYPE_COMMAND;
			putchar( c );

		} else if ( c == '\n' ){
			buf[i] = 0;
			found = 1;

			ret = malloc( sizeof( sh_token_t ));
			ret->str = buf;
			ret->type = TYPE_NEWLINE;
			putchar( c );
		}

		if ( c != '\b' ){
			i++;
		}
	}

	if ( !ret ){
		free( buf );
	}

	return ret;
}

sh_token_t *read_command( FILE *stream ){
	sh_token_t *ret;
	sh_token_t *move;

	ret = move = read_token( stream );

	while ( move && move->type != TYPE_NEWLINE ){
		move->next = read_token( stream );
		move = move->next;
	}

	return ret;
}

sh_token_t *dump_tokens( sh_token_t *tokens ){
	sh_token_t *move;

	for ( move = tokens; move; move = move->next ){
		printf( "[%s] Have token \"%s\"\n", __func__, move->str );
	}

	return tokens;
}

unsigned tokens_length( sh_token_t *tokens ){
	unsigned ret = 0;
	sh_token_t *move;

	for ( move = tokens; move; move = move->next ){
		ret++;
	}

	return ret;
}

parsed_command_t *parse_command( sh_token_t *tokens ){
	parsed_command_t *ret;
	sh_token_t *move;
	unsigned len;
	unsigned i;

	ret = malloc( sizeof( parsed_command_t ));
	ret->num_args = tokens_length( tokens );
	ret->args = malloc( sizeof( char *[ ret->num_args + 1]));

	move = tokens;
	for ( i = 0; i < ret->num_args; i++ ){
		ret->args[i] = move->str;
		move = move->next;
	}

	ret->args[i] = NULL;

	return ret;
}

int exec_cmd( parsed_command_t *cmd ){
	int pid;
	int ret = 0;

	if ( strlen( cmd->args[0] ) > 0 ){
		pid = _spawn( cmd->args[0], cmd->args, NULL );

		if ( pid > 0 ){
			syscall_waitpid( pid, &ret, 0 );
		} else {
			puts( "Command not found." );
		}
	}

	return ret;
}

int main( int argc, char *argv[], char *envp[] ){
	char *buf = malloc( 128 );
	int i, c;
	int running = 1;
	parsed_command_t *cmd;

	while ( running ){
		printf( "$ " );
		//cmd = parse_command( dump_tokens( read_command( stdin )));
		cmd = parse_command( read_command( stdin ));
		exec_cmd( cmd );
	}

	return 0;
}
