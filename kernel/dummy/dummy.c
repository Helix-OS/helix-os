#include <base/stdio.h>

char *depends[] = { "base", 0 };
char *provides = "dummy";

int init( ){
	kprintf( "[%s] Hello world!\n", provides );
	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
