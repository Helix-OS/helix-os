#ifndef _helix_cpplib_module_h
#define _helix_cpplib_module_h 1

#include <base/mem/alloc.h>

namespace klib {
	typedef unsigned long size_t;
	typedef unsigned nothrow_t;

	extern const nothrow_t &nothrow;

} // namespace klib

void *operator new( klib::size_t size ) throw( );
void *operator new[]( klib::size_t size ) throw( );
void *operator new( klib::size_t size, const klib::nothrow_t nothrow_value ) throw( );
void *operator new[]( klib::size_t size, const klib::nothrow_t nothrow_value ) throw( );

void operator delete( void *ptr ) throw( );
void operator delete[]( void *ptr ) throw( );

#endif
