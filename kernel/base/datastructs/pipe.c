#include <base/datastructs/pipe.h>
#include <base/mem/alloc.h>
#include <base/string.h>

pipe_t *pipe_create( unsigned outputs, unsigned buf_size, unsigned flags ){
	pipe_t *ret = 0, 
	       *temp;
	unsigned i;

	if ( outputs > 0 ){
		temp = kmalloc( sizeof( pipe_t ));
		temp->nbufs = outputs;

		temp->bufs = kmalloc( sizeof( pipeline_t *[outputs] ));

		for ( i = 0; i < outputs; i++ ){
			temp->bufs[i] = kmalloc( sizeof( pipeline_t ));
			memset( temp->bufs[i], 0, sizeof( pipeline_t ));

			temp->bufs[i]->size = buf_size;
			temp->bufs[i]->avail = 0;
			temp->bufs[i]->writep = 0;
			temp->bufs[i]->readp = 0;
			temp->bufs[i]->flags = PIPE_FLAG_OPEN | flags;

			temp->bufs[i]->buf = kmalloc( sizeof( char[buf_size] ));
		}

		ret = temp;
	}

	return ret;
}

int pipe_write( pipe_t *pipe, char *buf, unsigned size ){
	pipeline_t *line;
	unsigned i, j, c, ret = 0;

	for ( i = 0; i < pipe->nbufs; i++ ){
		line = pipe->bufs[i];
		//kprintf( "pipe_write: writing to pipe %d:0x%x\n", i, line );

		if ( line->flags & PIPE_FLAG_OPEN ){
			for ( j = 0; j < size; j++ ){
				if ( line->avail == line->size - 1 ){
					// Just return and assume the caller will either block,
					// try and call again, or decide it didn't really want to
					// send that data anyways
					break;

				} else {
					line->avail++;
				}

				c = ( line->writep ) % line->size;
				line->buf[c] = buf[j];
				ret++;
				line->writep++;
			}

			//kprintf( "pipe_write: pipe->bufs[%d]->buf = \"%s\", avail = %d\n", i, line->buf, line->avail );
		}
	}

	return ret;
}

int pipeline_read( pipeline_t *line, char *buf, unsigned size ){
	unsigned i, j;

	for ( i = 0; i < size && i < line->avail + i; i++, line->avail-- ){
		j = line->readp;

		buf[i] = line->buf[j];
		line->readp = ( line->readp + 1 ) % line->size;
	}

	return i;
}
