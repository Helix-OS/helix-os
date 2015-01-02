#include <base/logger.h>

extern "C" {
	const char *depends[] = { "base", 0 };
	const char *provides = "cppmod";

	int  init( );
	void remove( );
}

class something {
	public:
		something( const char *s ){
			str = s;
		};

		const char *getstr( ){ return str; };
		const char *str;
};

int init( ){
	something meh( "asdf" );
	auto &thing = meh;

	kprintf( "[%s] Hello world!\n", provides );
	kprintf( "[%s] This is module \"%s\", and I'm in yo kernelz\n",
			provides, provides );

	kprintf( "[%s] have %s\n", provides, thing.getstr( ) );

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
