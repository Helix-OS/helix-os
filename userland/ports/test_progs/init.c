#include <dalibc/syscalls.h>
#include <stdio.h>
#include <stdlib.h>

static char *text_init_commands[] = {
	"/bin/console",
	NULL,
};

static char *graphic_init_commands[] = {
	"/bin/fbman",
	NULL,
};

void print_nifty_ascii_art( void ){
	puts(
		"    _________    _________     \n"
		"   / _____  /\\  / _____  /\\     _          __    _\n"
		"  / /____/ / _\\/ /____/ /  \\   / |_   ___ |_ |  |_|  _ _\n"
		" /________/ /_/________/ /\\ \\  |   \\ | - | | |   _  / ' \\\n"
		" \\  _____ \\ \\_\\  _____ \\ \\/ /  |   | |  -\\ | ', | | >   <\n"
		"  \\ \\____\\ \\  /\\ \\____\\ \\  /   \\_|_| \\___/ \\__/ |_/ \\_'_/\n"
		"   \\________\\/  \\________\\/                            \n"
	);
}

int main( int argc, char *argv[], char *envp[] ){
	int i;
	char *asdf = malloc( 32 );

	print_nifty_ascii_art( );
	/*
	syscall_chroot( userroot );
	printf( "Changed root to %s\n", userroot );
	*/

	char **init_commands = NULL;

	if ( argc > 1 && strcmp( argv[1], "graphic" ) == 0 ){
		init_commands = graphic_init_commands;
	} else {
		init_commands = text_init_commands;
	}

	for ( i = 0; init_commands[i]; i++ ){
		printf( "starting %s...\n", init_commands[i] );
		_spawn( init_commands[i], NULL, NULL, NULL );
	}

	return 0;
}
