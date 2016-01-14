#ifndef _helix_datastruct_pipe
#define _helix_datastruct_pipe
#include <base/tasking/task.h>

enum {
	PIPE_FLAG_NULL,
	PIPE_FLAG_OPEN,
	PIPE_FLAG_BLOCK,
};

typedef struct pipeline {
	unsigned size;
	unsigned avail;
	unsigned writep;
	unsigned readp;
	unsigned flags;

	char *buf;
} pipeline_t;

typedef struct pipe {
	unsigned nbufs;
	unsigned buf_size;
	unsigned used;
	unsigned readers;
	unsigned writers;
	semaphore_t sem;

	pipeline_t **bufs;
} pipe_t;

pipe_t *pipe_create( unsigned outputs, unsigned buf_size, unsigned flags );
int pipe_write( pipe_t *pipe, char *buf, unsigned size );
int pipeline_read( pipeline_t *line, char *buf, unsigned size );

#endif
