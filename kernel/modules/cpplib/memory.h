#ifndef _helix_cpplib_memory_module_h
#define _helix_cpplib_memory_module_h 1
#include <cpplib/cpplib.h>

namespace klib {
	template<class T>
	class default_delete {
		public:
			const void operator( )( T *ptr ){
				delete ptr;
			}
	};

	template<class T, class Deleter = default_delete<T>>
	class unique_ptr {
		public:
			unique_ptr( ){
				empty = true;
				uniq = 0;
			};

			unique_ptr( T *ptr ){
				uniq = ptr;
				empty = false;
			};

			~unique_ptr( ){
				reset( );
			};

			T *get( ){
				return uniq;
			};

			T *release( ){
				T *ret = uniq;

				uniq = 0;
				empty = true;

				return ret;
			}

			void reset( ){
				if ( !empty ){
					deleter.operator()( uniq );
					uniq = 0;
				}
			}

			void reset( T *ptr ){
				reset( );

				uniq = ptr;
				empty = false;
			}

			T *operator*( ){
				return uniq;
			};

			T *operator->( ){
				return uniq;
			};

			unique_ptr& operator=( unique_ptr &&ptr ){
				unique_ptr &ret = *this;

				reset( ptr.release( ));
				deleter = ptr.deleter;

				kprintf( "[%s] Moved ownership of pointer 0x%x...\n", __func__, uniq );

				return ret; 
			};

			// TODO: implement add_lvalue_reference so this will work right
			T &operator[](size_t i){
				return get( )[i];
			}

		private:
			Deleter deleter;
			bool empty;
			T *uniq;
	};

	// "unique" as an alias of "unique_ptr"
	template<class T, class Deleter = default_delete<T>>
	using unique = unique_ptr<T, Deleter>;

} // namespace klib;

#endif
