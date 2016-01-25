#include <dalibc/syscalls.h>
#include <stdio.h>
#include <stdlib.h>

static char *init_commands[] = {
	//"/bin/console",
	"/helix/userroot/bin/fbman",
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
	char *userroot = "/helix/userroot";

	print_nifty_ascii_art( );
	/*
	syscall_chroot( userroot );
	printf( "Changed root to %s\n", userroot );
	*/

	for ( i = 0; init_commands[i]; i++ ){
		printf( "starting %s...\n", init_commands[i] );
		_spawn( init_commands[i], NULL, NULL, NULL );
	}

	return 0;
}
