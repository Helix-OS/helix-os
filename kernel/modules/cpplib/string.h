#ifndef _helix_cpplib_string_module_h
#define _helix_cpplib_string_module_h 1
#include <cpplib/cpplib.h>

namespace klib {
	class string {
		public:
			string( );
			string( const char *s );
			~string( );

			string &assign( const char *s );
			string &assign( const string &str );

			string &operator= (const string &str);
			string &operator= (const char *s );
			string &operator= (char c);

			const char *c_str( ) const;

		private:
			size_t size;
			char *buf;
	};

} // namespace klib;

#endif
