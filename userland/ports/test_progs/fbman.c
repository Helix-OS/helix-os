#include <stdio.h>
#include <dalibc/syscalls.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct psf2_header {
    uint8_t  magic[4];
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t length;
    uint32_t charsize;
    uint32_t height;
    uint32_t width;
} __attribute__((packed)) psf2_header_t;

typedef struct framebuf {
	uint32_t *addr;
	unsigned height;
	unsigned width;
	int fd;
	int keyboard;
	int mouse;
	unsigned redraw;
	psf2_header_t *font;
} framebuf_t;

enum {
	W_EVENT_NULL,
	W_EVENT_KEY,
	W_EVENT_MOUSE,
	W_EVENT_MOVE,
	W_EVENT_RESIZE,
};

typedef struct winevent {
	unsigned type;
	unsigned data[4];

	struct winevent *next;
} winevent_t;

typedef struct window window_t;
typedef void (*window_update)( window_t *window, framebuf_t *fb );
typedef void (*window_draw)( window_t *window, framebuf_t *fb );

struct window {
	char *title;
	unsigned width;
	unsigned height;
	unsigned xpos;
	unsigned ypos;
	unsigned focused;

	window_update update;
	window_draw   draw;

	window_t *prev;
	window_t *next;

	winevent_t *events;

	void *progdata;
};

#define FOR_EACH_WINDOW_EVENT( EVENT_VAR, WINDOW ) \
	for ( (EVENT_VAR) = window_get_event((WINDOW)); \
	      (EVENT_VAR); \
	      (EVENT_VAR) = window_get_event((WINDOW)) )

void *read_file( char *path ){
	int fd = open( path, 0 );
	char *ret = NULL;

	if ( fd > 0 ){
		int size;

		lseek( fd, 0, SEEK_END );
		size = lseek( fd, 0, SEEK_CUR );
		lseek( fd, 0, SEEK_SET );

		ret = sbrk( sizeof( char[size + 1]));
		read( fd, ret, size );
		close( fd );
	}

	return ret;
}

framebuf_t *open_framebuffer( char *fbpath, char *keyboard, char *mouse ){
	framebuf_t *fb = calloc( 1, sizeof( framebuf_t ));

	fb->fd = open( fbpath, 0 );
	fb->keyboard = open( keyboard, 0 );
	fb->mouse = -1;

	fcntl( fb->keyboard, F_SETFL, O_NONBLOCK );

	fb->width  = 1024;
	fb->height = 768;
	fb->redraw = 1;
	fb->addr   = sbrk( sizeof( uint32_t[fb->width * fb->height]));
	fb->font   = read_file( "/helix/userroot/etc/fbman_font.psf" );

	return fb;
}

int redraw_framebuffer( framebuf_t *fb ){
	lseek( fb->fd, 0, SEEK_SET );

	return write( fb->fd, fb->addr, sizeof( uint32_t[fb->width * fb->height]));
}

int draw_box( framebuf_t *fb, unsigned xpos, unsigned ypos,
			  unsigned width, unsigned height, uint32_t color )
{
	unsigned x = xpos;
	unsigned y = ypos;
	unsigned i = 0;
	unsigned k = 0;

	for ( k = 0; k < height && y < fb->height; y++, k++ ){
		x = xpos;
		for ( i = 0; i < width && x < fb->width; x++, i++ ){
			*(fb->addr + (y * fb->width) + x) = color;
		}
	}

	fb->redraw = 1;

	return 0;
}

void draw_char( framebuf_t *fb, unsigned c, unsigned xpos, unsigned ypos, unsigned color ){
	psf2_header_t *font = fb->font;
	uint8_t *bitmap = (uint8_t *)font + font->header_size;
	unsigned x, y;
	unsigned mod = 1 << (font->width - 1);
	uint32_t *place = fb->addr + (ypos * fb->width) + xpos;

	for ( y = 0; y < font->height; y++ ){
		for ( x = 0; x < font->width; x++ ){
			if ( bitmap[c * font->charsize + y] & (mod >> x)){
				*place = color;
			}

			place += 1;
		}

		place += fb->width - font->width;
	}
}

void draw_string( framebuf_t *fb, char *str, unsigned xpos, unsigned ypos, unsigned color ){
	unsigned place = xpos;
	unsigned i;

	for ( i = 0; str[i] && place < fb->width; i++, place += fb->font->width ){
		draw_char( fb, str[i], place, ypos, color );
	}
}

void window_update_do_nothing( window_t *window, framebuf_t *fb ){
	// dutifully do nothing
}

void window_draw_nothing( window_t *window, framebuf_t *fb ){
	// this function intentionally left blank
}

window_t *make_window( char *title, unsigned xpos, unsigned ypos,
					   unsigned width, unsigned height, window_t *next )
{
	window_t *ret = calloc( 1, sizeof( window_t ));

	ret->title = title;
	ret->xpos = xpos;
	ret->ypos = ypos;
	ret->width = width;
	ret->height = height;
	ret->update = window_update_do_nothing;
	ret->draw = window_draw_nothing;
	ret->next = next;
	ret->focused = 0;
	ret->progdata = 0;

	if ( next ){
		next->prev = ret;
	}

	return ret;
}

void window_draw_string( framebuf_t *fb, window_t *window, char *str,
						 unsigned xpos, unsigned ypos, unsigned color )
{
	unsigned place = xpos + window->xpos;
	unsigned i;
	unsigned win_limit = window->width + window->xpos;

	for ( i = 0;
		  str[i]
		  && (place + fb->font->width) < fb->width
		  && (place + fb->font->width) < win_limit;
		  i++, place += fb->font->width )
	{
		draw_char( fb, str[i], place, ypos + window->ypos, color );
	}
}

void draw_window( framebuf_t *fb, window_t *window ){
	unsigned boxborder = 4;

	draw_box( fb, window->xpos - boxborder, window->ypos - boxborder - 16,
			  window->width + boxborder * 2,
			  16,
			  0x00303030 );

	//draw_char( fb, '#', window->xpos - boxborder + 2, window->ypos - boxborder - 15 + 2, 0xa0a0a0 );
	window_draw_string( fb, window, window->title,
		 0,  -boxborder - 16 + 2, 0xa0a0a0 );

	draw_box( fb, window->xpos - boxborder, window->ypos - boxborder,
			  window->width + (boxborder * 2), 
			  window->height + (boxborder * 2),
			  window->focused? 0x707070 : 0x505050 );
	draw_box( fb, window->xpos, window->ypos, window->width, window->height, 0xb8b8b8 );

	window->draw( window, fb );
}

void update_window( framebuf_t *fb, window_t *window ){
	if ( window->update ){
		window->update( window, fb );
	}
}

void resize_window( framebuf_t *fb, window_t *window, int xinc, int yinc ){
	unsigned boxborder = 4;
	draw_box( fb, window->xpos - boxborder * 3, window->ypos - boxborder * 3 - 16,
			  window->width + (boxborder * 6), 
			  window->height + (boxborder * 6) + 16,
			  0x405040 );

	window->width += xinc;
	window->height += yinc;
}

void move_window( framebuf_t *fb, window_t *window, int xinc, int yinc ){
	unsigned boxborder = 4;
	draw_box( fb, window->xpos - boxborder * 3, window->ypos - boxborder * 3 - 16,
			  window->width + (boxborder * 6), 
			  window->height + (boxborder * 6) + 16,
			  0x405040 );

	window->xpos += xinc;
	window->ypos += yinc;
}

void draw_window_list( framebuf_t *fb, window_t *list ){
	window_t *temp = list;

	for ( ; temp; temp = temp->next ){
		draw_window( fb, temp );
	}
}

void update_window_list( framebuf_t *fb, window_t *list ){
	window_t *temp = list;

	for ( ; temp; temp = temp->next ){
		update_window( fb, temp );
	}
}

window_t *focused_window = NULL;

window_t *focus_window( window_t *window ){
	window_t *ret;

	if ( window->next ){
		window_t *temp = window;
		for ( ; temp->next; temp = temp->next );

		temp->focused = 0;

		if ( window->prev ){
			window->prev->next = window->next;
		}

		if ( window->next ){
			window->next->prev = window->prev;
		}

		window->prev = temp;
		window->next = NULL;

		if ( window->prev ){
			window->prev->next = window;
		}
	}

	window->focused = 1;
	focused_window = window;
	ret = window;
	for ( ; ret->prev; ret = ret->prev );

	return ret;
}

void window_send_event( window_t *window, winevent_t *event ){
	if ( window->events ){
		event->next = window->events;
	}

	window->events = event;
}

winevent_t *window_get_event( window_t *window ){
	winevent_t *ret = NULL;

	if ( window->events ){
		ret = window->events;
		window->events = ret->next;
		ret->next = NULL;
	}

	return ret;
}

winevent_t *make_keyboard_event( char ch ){
	winevent_t *ret = calloc( 1, sizeof( winevent_t ));

	ret->type = W_EVENT_KEY;
	ret->data[0] = ch;

	return ret;
}

void test_printer_update( window_t *window, framebuf_t *fb ){
	winevent_t *ev;
	unsigned *progdata = window->progdata;

	FOR_EACH_WINDOW_EVENT( ev, window ){
		if ( ev->type == W_EVENT_KEY ){
			if ( ev->data[0] == '\n' ){
				*progdata = 1;

			} else {
				*progdata = *progdata + 1;
			}
		}

		free( ev );
	}
}

void test_printer_draw( window_t *window, framebuf_t *fb ){
	unsigned i;
	unsigned k;
	unsigned *numkeys = window->progdata;

	for ( k = i = 0;
		  k < *numkeys && (i + fb->font->height) < window->height;
		  i += fb->font->height, k++ )
	{
		window_draw_string( fb, window, "Testing this thing...\n", 0, i, 0x202020 );
	}
}

window_t *make_test_printer( window_t *winlist ){
	window_t *ret = make_window( "A test window", 400, 400, 240, 180, winlist );
	ret->draw = test_printer_draw;
	ret->update = test_printer_update;
	ret->progdata = malloc( sizeof( unsigned ));

	unsigned *foo = ret->progdata;
	*foo = 3;

	return ret;
}

typedef struct term_data {
	int pid;
	int out_fd;
	int in_fd;

	char *textbuf;
	unsigned text_xpos;
	unsigned text_ypos;
	unsigned text_height;
	unsigned text_width;

	char *linebuf;
	unsigned line_len;
	unsigned line_pos;
} term_data_t;

void terminal_put_char( term_data_t *term, char ch ){
	unsigned i;

	switch ( ch ){
		case '\n':
			term->text_ypos++;
		case '\r':
			term->text_xpos = 0;
			return;

		case '\b':
			term->text_xpos--;
			i = term->text_ypos * term->text_width + term->text_xpos;
			term->textbuf[i] = '\0';
			return;

		default:
			break;

	}

	if ( term->text_xpos == term->text_width ){
		term->text_xpos = 0;
		term->text_ypos++;
	}

	if ( term->text_ypos >= term->text_height - 1 ){
		for ( i = 0; i < term->text_height - 1; i++ ){
			memcpy( term->textbuf + i * term->text_width,
					term->textbuf + (i + 1) * term->text_width,
					term->text_width );
		}

		term->text_ypos = term->text_height - 2;
	}

	i = term->text_ypos * term->text_width + term->text_xpos;
	term->textbuf[i] = ch;
	term->text_xpos++;
}

char *terminal_get_line( term_data_t *term, char ch ){
	char *ret = NULL;

	if ( ch == '\n' ){
		if ( term->line_pos ){
			terminal_put_char( term, '\b' );
		}

		term->linebuf[term->line_pos] = ch;
		term->linebuf[term->line_pos + 1] = '\0';
		term->line_pos = 0;
		ret = term->linebuf;
		terminal_put_char( term, '\n' );

	} else if ( ch == '\b' ){
		if ( term->line_pos ){
			term->line_pos--;
			term->linebuf[term->line_pos] = '\b';
			terminal_put_char( term, '\b' );

			terminal_put_char( term, '\b' );
			terminal_put_char( term, '_' );
		}

	} else {
		if ( term->line_pos ){
			terminal_put_char( term, '\b' );
		}

		term->linebuf[term->line_pos] = ch;
		term->line_pos++;
		terminal_put_char( term, ch );
		terminal_put_char( term, '_' );
	}

	return ret;
}

void terminal_update( window_t *window, framebuf_t *fb ){
	term_data_t *term = window->progdata;
	winevent_t *ev;
	char ch;

	FOR_EACH_WINDOW_EVENT( ev, window ){
		switch( ev->type ){
			case W_EVENT_KEY:
				{
					char *line;
					ch = ev->data[0];

					if (( line = terminal_get_line( term, ch ))){
						write( term->out_fd, line, strlen( line ));
					}
				}

				break;

			default:
				break;
		}

		free( ev );
	}

	int inret = read( term->in_fd, &ch, 1 );

	while ( inret > 0 ){
		terminal_put_char( term, ch );
		inret = read( term->in_fd, &ch, 1 );
	}
}

void terminal_draw( window_t *window, framebuf_t *fb ){
	term_data_t *term = window->progdata;
	unsigned x, y;

	for ( y = 0; y < term->text_height; y++ ){
		for ( x = 0; x < term->text_width; x++ ){
			if ( *(term->textbuf + y * term->text_width + x)) {
				draw_char( fb,
						   *(term->textbuf + y * term->text_width + x),
						   //*(term->textbuf),
						   //window->xpos,
						   //window->ypos,
						   window->xpos + 2 + x * fb->font->width,
						   window->ypos + 2 + y * fb->font->height,
						   //window->ypos,
						   0x202020 );
			}
		}
	}
}

window_t *make_terminal( framebuf_t *fb, window_t *winlist ){
	window_t *ret = make_window( "Terminal", 75, 95, 4 + 80 * fb->font->width,
								 4 + 25 * fb->font->height, winlist );
	ret->draw = terminal_draw;
	ret->update = terminal_update;
	term_data_t *term;
	int to[2];
	int from[2];

	term = ret->progdata = calloc( 1, sizeof( term_data_t ));

	pipe( to );
	pipe( from );
	int p_fds[3] = { to[0], from[1], from[1] };
	term->in_fd   = from[0];
	term->out_fd  = to[1];

	term->textbuf = sbrk( sizeof( char[80 * 26] ));
	memset( term->textbuf, 0, sizeof( char[80 * 26] ));
	term->text_height = 26;
	term->text_width = 80;
	//term->pid = _spawn( "/test/userroot/bin/sh", NULL, NULL, p_fds );

	term->linebuf = malloc( sizeof( char[128] ));
	term->line_pos = 0;
	term->line_len = 128;

	term->pid = _spawn( "/bin/sh", NULL, NULL, p_fds );

	close( to[0] );
	close( from[1] );
	fcntl( to[1],   F_SETFL, O_NONBLOCK );
	fcntl( from[0], F_SETFL, O_NONBLOCK );

	return ret;
}

int main( int argc, char *argv[], char *envp[] ){
	printf( "[fbman] Opening framebuffer...\n" );
	framebuf_t *fb = open_framebuffer( "/helix/devices/framebuffer",
	                                   "/helix/devices/keyboard",
	                                   NULL );

	syscall_chroot( "/helix/userroot" );

	printf( "[fbman] Creating some testing windows...\n" );
	window_t *testwin = NULL;
	/*
	testwin = make_window( "# testing this", 300, 300, 200, 150, NULL );
	testwin = make_window( "Another window", 550, 550, 150, 200, testwin );
	testwin = make_window( "even moar windows", 500, 200, 150, 200, testwin );
	testwin = make_test_printer( testwin );
	*/
	//testwin = make_test_printer( testwin );
	testwin = make_terminal( fb, testwin );
	testwin = focus_window( testwin );

	printf( "[fbman] Filling in background...\n" );

	{
		unsigned x = 0;
		unsigned y = 0;

		for ( ; y < fb->height; y++ ){
			x = 0;
			for ( ; x < fb->width; x++ ){
				*(fb->addr + (y * fb->width) + x) = 0x405040;
			}
		}
	}

	printf( "[fbman] Initialization done, entering draw loop\n" );

	//draw_char( fb, '#', 700, 300, 0x202020 );

	for ( ;; ){
		/*
		if ( wut ){
			memset( fb->addr, 0xff, sizeof( uint32_t[1024*768] ));
		} else {
			memset( fb->addr, 0x80, sizeof( uint32_t[1024*768] ));
		}
		*/

		update_window_list( fb, testwin );

		//draw_box( fb, 50, 70, 100, 100, 0x00806080 );
		//draw_window( fb, testwin );
		draw_window_list( fb, testwin );
		draw_box( fb, 0, 0, 1024, 20, 0x00303030 );
		draw_string( fb, "[1] [2] [3] [4] asdf ", 4, 4, 0xa0a0a0 );

		redraw_framebuffer( fb );

		//int c = getchar( );
		unsigned char c;

		if ( read( fb->keyboard, &c, 1 ) > 0 ) {
			if ( c == '\x1b' ){
				getchar( );
				c = getchar( );

				switch ( c ){
					case 'h':
						//boxpos_x -= 40;
						move_window( fb, focused_window, -40, 0 );
						break;
					case 'l':
						move_window( fb, focused_window, 40, 0 );
						//boxpos_x += 40;
						break;
					case 'j':
						move_window( fb, focused_window, 0, 40 );
						//boxpos_y += 40;
						break;
					case 'k':
						move_window( fb, focused_window, 0, -40 );
						//boxpos_y -= 40;
						break;
					case 'J':
						//boxheight += 40;
						resize_window( fb, focused_window, 0, 40 );
						break;
					case 'K':
						resize_window( fb, focused_window, 0, -40 );
						//boxheight -= 40;
						break;
					case 'H':
						resize_window( fb, focused_window, -40, 0 );
						//boxwidth -= 40;
						break;
					case 'L':
						resize_window( fb, focused_window, 40, 0 );
						//boxwidth += 40;
						break;

					case 'C':
						//testwin = make_window( "even moar windows", 500, 200, 150, 200, testwin );
						testwin = make_terminal( fb, testwin );
						testwin = focus_window( testwin );
						break;

					case '\t':
						if ( testwin ){
							testwin = focus_window( testwin );
						}
						break;
						/*
						   case 'M':
						   boxborder -= 1;
						   break;
						   case 'N':
						   boxborder += 1;
						   break;
						   */
					default:
						break;
				}

			} else {
				window_send_event( focused_window, make_keyboard_event( c ));
			}
		}
	}
}
