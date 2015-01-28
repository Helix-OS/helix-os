#include <base/logger.h>
#include <base/string.h>

#include <cpplib/cpplib.h>
#include <cpplib/memory.h>

extern "C" {
	const char *depends[] = { "base", 0 };
	const char *provides = "cpplib";

	int  init( );
	void remove( );
}

class something {
	public:
		something( const char *s ){
			str = s;
		};

		~something( ){
			kprintf( "[%s] Got here, have string \"%s\"\n", __func__, str );
		}

		const char *getstr( ){ return str; };
		const char *str;
};

void *operator new( klib::size_t size ) throw( ){
	// TODO: handle out-of-memory errors, once exceptions are implemented
	return kmalloc( size );
}

void *operator new[]( klib::size_t size ) throw( ){
	// TODO: handle out-of-memory errors, once exceptions are implemented
	return kmalloc( size );
}

void *operator new( klib::size_t size, const klib::nothrow_t nothrow_value ) throw( ){
	return kmalloc( size );
}

void *operator new[]( klib::size_t size, const klib::nothrow_t nothrow_value ) throw( ){
	return kmalloc( size );
}

void operator delete( void *ptr ) throw(){
	kprintf( "[%s] Got here\n", __func__ );
	kfree( ptr );
}

void operator delete[]( void *ptr ) throw(){
	kfree( ptr );
}

something *test_func(){
	something *ret = new something( "Testing unique pointer movement between functions" );

	kprintf( "[%s] Ello, returning unique pointer with \"%s\"\n", __func__, ret->str );

	return ret;
}

void print_something( const something *foo ){
	kprintf( "[%s] Have \"%s\"\n", __func__, foo->str );
}

const klib::nothrow_t &klib::nothrow = 1;

int init( ){
	kprintf( "[%s] Hello world!\n", provides );
	kprintf( "[%s] This is module \"%s\", and I'm in yo kernelz\n", provides, provides );

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
