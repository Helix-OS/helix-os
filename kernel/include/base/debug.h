#ifndef _helix_debug_h
#define _helix_debug_h
#include <base/stdarg.h>
#include <base/logger.h>

// TODO: Add comments describing the structures here once this is finished

typedef enum debug_filter {
	DEBUG_NONE     = 0,
	DEBUG_CORE     = 1,
	DEBUG_TASKING  = 2,
	DEBUG_ALLOC    = 4,
	DEBUG_MODULE   = 8,
	DEBUG_VFS      = 16,
	DEBUG_IO       = 32,
	DEBUG_ALL      = 0xffffffff,

	DEBUG_FILTER   = DEBUG_ALL,
} debug_filt_t;

typedef enum debug_mask {
	MASK_NONE       = 0,
	MASK_VERBOSE    = 1,
	MASK_DEVINFO    = 2,
	MASK_CHECKPOINT = 4,
	MASK_ALL        = 0xffffffff,

	DEBUG_MASK      = MASK_NONE,
} debug_mask_t;

static inline int debugp( debug_filt_t filter, debug_mask_t mask, char *fmt, ... ){
	if (( filter & DEBUG_FILTER ) && !( mask & DEBUG_MASK )){
		va_list va;
		int i;

		va_start( va, fmt );
		i = kvprintf( fmt, va );
		va_end( va );

		return i;

	} else {
		return 0;
	}
}

#endif
