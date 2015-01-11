#include <base/logger.h>
#include <base/string.h>

#include <cpplib/cpplib.h>
#include <cpplib/string.h>

using namespace klib;

string::string( ){
	buf = 0;
	size = 0;
}

string::~string( ){
	kprintf( "[%s] decontructing string...\n", __func__ );
	delete buf;
}

string::string( const char *s ){
	buf = 0;
	size = 0;

	assign( s );
}

const char *string::c_str( ) const {
	return buf;
}

string &string::assign( const char *s ){
	if ( buf ){
		delete buf;
		buf = 0;
	}

	size = strlen( s );

	if ( size ){
		buf = new char[size + 1];
		strcpy( buf, s );
	}

	return *this;
}

string &string::assign( const string &str ){
	assign( str.c_str( ));
	return *this;
}

string &string::operator= (const string &str){
	kprintf( "[%s] Moving stuff (string &)\n", __func__ );
	assign( str );
	return *this;
}

string &string::operator= (const char *s ){
	kprintf( "[%s] Moving stuff (char *)\n", __func__ );
	return assign( s );
}
