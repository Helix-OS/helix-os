#ifndef _helix_ipc_pipes_h
#define _helix_ipc_pipes_h
#include <base/tasking/pobj.h>
#include <base/vfs/vfs.h>
#include <base/datastructs/shared.h>

#define PIPE_POBJ     0xcafebeef
#define PIPELINE_POBJ 0xbeefcafe

// this handles both the reading and writing ends
typedef struct pipe_pobj {
	base_pobj_t base;
	shared_t *pipe;
	unsigned flags;
} pipe_pobj_t;

file_pobj_t *make_pipe_pobj( );
int make_pipes( int *fds );

#endif
