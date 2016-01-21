#include <stdio.h>
#include <dalibc/syscalls.h>
#include <stdint.h>
#include <stdlib.h>

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
};

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

	fb->fd = open( "/test/devices/framebuffer", 0 );
	fb->keyboard = open( "/test/devices/keyboard", 0 );
	fb->mouse = -1;

	fb->width  = 1024;
	fb->height = 768;
	fb->redraw = 1;
	fb->addr   = sbrk( sizeof( uint32_t[fb->width * fb->height]));
	fb->font   = read_file( "/test/userroot/etc/fbman_font.psf" );

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

	if ( next ){
		next->prev = ret;
	}

	return ret;
}

void draw_window( framebuf_t *fb, window_t *window ){
	unsigned boxborder = 4;

	draw_box( fb, window->xpos - boxborder, window->ypos - boxborder - 16,
			  window->width + boxborder * 2,
			  16,
			  0x00303030 );

	//draw_char( fb, '#', window->xpos - boxborder + 2, window->ypos - boxborder - 15 + 2, 0xa0a0a0 );
	draw_string( fb, window->title,
		 window->xpos, window->ypos - boxborder - 16 + 2, 0xa0a0a0 );

	draw_box( fb, window->xpos - boxborder, window->ypos - boxborder,
			  window->width + (boxborder * 2), 
			  window->height + (boxborder * 2),
			  window->focused? 0x707070 : 0x505050 );
	draw_box( fb, window->xpos, window->ypos, window->width, window->height, 0x00c0c0c0 );
}

void resize_window( framebuf_t *fb, window_t *window, int xinc, int yinc ){
	unsigned boxborder = 4;
	draw_box( fb, window->xpos - boxborder * 3, window->ypos - boxborder * 3 - 16,
			  window->width + (boxborder * 6), 
			  window->height + (boxborder * 6) + 16,
			  0x608060 );

	window->width += xinc;
	window->height += yinc;
}

void move_window( framebuf_t *fb, window_t *window, int xinc, int yinc ){
	unsigned boxborder = 4;
	draw_box( fb, window->xpos - boxborder * 3, window->ypos - boxborder * 3 - 16,
			  window->width + (boxborder * 6), 
			  window->height + (boxborder * 6) + 16,
			  0x608060 );

	window->xpos += xinc;
	window->ypos += yinc;
}

void draw_window_list( framebuf_t *fb, window_t *list ){
	window_t *temp = list;

	for ( ; temp; temp = temp->next ){
		draw_window( fb, temp );
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

int main( int argc, char *argv[], char *envp[] ){
	framebuf_t *fb = open_framebuffer( "/test/devices/fbconsole",
	                                   "/test/devices/keyboard",
	                                   NULL );

	window_t *testwin = make_window( "# testing this", 300, 300, 200, 150, NULL );
	testwin = make_window( "Another window", 550, 550, 150, 200, testwin );
	testwin = make_window( "even moar windows", 500, 200, 150, 200, testwin );
	testwin = focus_window( testwin );

	//memset( fb->addr, 0x80, sizeof( uint32_t[1024*768] ));
	{
		unsigned x = 0;
		unsigned y = 0;

		for ( ; y < fb->height; y++ ){
			x = 0;
			for ( ; x < fb->width; x++ ){
				*(fb->addr + (y * fb->width) + x) = 0x608060;
			}
		}
	}

	//draw_char( fb, '#', 700, 300, 0x202020 );

	for ( ;; ){
		/*
		if ( wut ){
			memset( fb->addr, 0xff, sizeof( uint32_t[1024*768] ));
		} else {
			memset( fb->addr, 0x80, sizeof( uint32_t[1024*768] ));
		}
		*/


		draw_box( fb, 50, 70, 100, 100, 0x00806080 );
		//draw_window( fb, testwin );
		draw_window_list( fb, testwin );
		draw_box( fb, 0, 0, 1024, 20, 0x00303030 );
		draw_string( fb, "[1] [2] [3] [4] asdf ", 4, 4, 0xa0a0a0 );

		redraw_framebuffer( fb );

		int c = getchar( );
		switch ( c ){
			case 'h':
				//boxpos_x -= 10;
				move_window( fb, focused_window, -10, 0 );
				break;
			case 'l':
				move_window( fb, focused_window, 10, 0 );
				//boxpos_x += 10;
				break;
			case 'j':
				move_window( fb, focused_window, 0, 10 );
				//boxpos_y += 10;
				break;
			case 'k':
				move_window( fb, focused_window, 0, -10 );
				//boxpos_y -= 10;
				break;
			case 'J':
				//boxheight += 10;
				resize_window( fb, focused_window, 0, 10 );
				break;
			case 'K':
				resize_window( fb, focused_window, 0, -10 );
				//boxheight -= 10;
				break;
			case 'H':
				resize_window( fb, focused_window, -10, 0 );
				//boxwidth -= 10;
				break;
			case 'L':
				resize_window( fb, focused_window, 10, 0 );
				//boxwidth += 10;
				break;

			case 'C':
				testwin = make_window( "even moar windows", 500, 200, 150, 200, testwin );
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
	}
}
