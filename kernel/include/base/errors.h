#ifndef _helix_errors_h
#define _helix_errors_h

typedef enum error_number {
	ERROR_NULL,

	/* General errors */
	ERROR_NO_FUNC,

	/* VFS errors */
	ERROR_NOT_FOUND,
	ERROR_INVALID_PATH,
} error_number_t;

#endif
