#include <vbe/vbe.h>
#include <base/kstd.h>
#include <base/hal.h>
#include <base/logger.h>
#include <base/string.h>
#include <base/stdio.h>
#include <base/multiboot.h>
#include <base/initrd.h>

char *depends[] = { "base", "hal", 0 };
char *provides = "vbe";

// defined in base/arch/i586/arch_init.c
// TODO: wrap this in a function or something
extern multiboot_header_t *bootheader;
// define in base/kmain.c
extern initrd_t *main_initrd;

static int vbe_console_hal_write( struct hal_device *dev, void *buf, unsigned count, unsigned offset );
static int vbe_framebuf_hal_write( struct hal_device *dev, void *buf, unsigned count, unsigned offset );

static void set_pixel( vbe_device_t *dev, unsigned x, unsigned y, uint32_t val ){
	if ( x < dev->x_res && y < dev->y_res ){
		uint32_t *place = (uint32_t *)(dev->framebuf + y * dev->pitch + x * 4);
		*place = val;
	}
}

static void put_char( vbe_device_t *dev, unsigned c, unsigned xpos, unsigned ypos ){
	psf2_header_t *font = dev->fontfile;
	uint8_t *bitmap = (uint8_t *)font + dev->fontfile->header_size;
	unsigned x, y;
	unsigned mod = 1 << (font->width - 1);
	uint32_t *place = (uint32_t *)(dev->framebuf + ypos * dev->pitch + xpos * 4);

	for ( y = 0; y < font->height; y++ ){
		for ( x = 0; x < font->width; x++ ){
			if ( bitmap[c * font->charsize + y] & (mod >> x)){
				*place = VBE_FOREGROUND;

			} else {
				*place = VBE_BACKGROUND;
			}

			place += 1;
		}

		place += dev->x_res - font->width;
	}
}

static void redraw_textbuf( vbe_device_t *dev, unsigned startx, unsigned starty ){
	psf2_header_t *font = dev->fontfile;
	uint8_t *buf = dev->textbuf;
	unsigned x, y, c;

	for ( y = starty; y < dev->text_y; y++ ){
		for ( x = startx; x < dev->text_x; x++ ){
			c = buf[y * dev->text_x + x];

			if ( c ){
				put_char( dev, c, x * font->width, y * font->height );
			}
		}
	}
}

static void clear_screen( vbe_device_t *dev ){
	unsigned x, y;
	for ( y = 0; y < dev->mode->Yres; y++ ){
		for ( x = 0; x < dev->mode->Xres; x++ ){
			set_pixel( dev, x, y, VBE_BACKGROUND );
		}
	}

	dev->cur_y = dev->cur_x = 0;
}

static int vbe_framebuf_hal_write( struct hal_device *dev, void *buf, unsigned count, unsigned offset ){
	vbe_device_t *vbe = dev->dev;
	int ret = 0;

	unsigned fbufsize = vbe->x_res * 4 + vbe->y_res * vbe->pitch;
	unsigned nbytes = count;

	if ( count + offset > fbufsize ){
		nbytes = count - ((count + offset) - fbufsize);
	}

	memcpy( vbe->framebuf, buf, nbytes );
	ret = nbytes;

	return ret;
}

/* Offset is ignored in this function, since it's handled by the driver */
static int vbe_console_hal_write( struct hal_device *dev, void *buf, unsigned count, unsigned offset ){
	vbe_device_t *vbe = dev->dev;
	uint8_t *textbuf = vbe->textbuf;
	char *str = buf;
	int ret = 0;
	int i;
	unsigned startx, starty;

	starty = vbe->cur_y;
	startx = 0;

	for ( ret = 0; ret < count; ret++ ){
		switch ( str[ret] ){
			case '\n':
				vbe->cur_y++;
				starty = starty? starty - 1 : starty;
			case '\r':
				vbe->cur_x = 0;
				continue;
			case '\b':
				vbe->cur_x--;

				i = vbe->cur_x + vbe->cur_y * vbe->text_x;
				textbuf[i] = '\b';

				//set_cursor( dev, vbe->cur_x, vbe->cur_y );
				continue;
			default:
				break;
		}

		if ( vbe->cur_x == vbe->text_x ){
			vbe->cur_x = 0;
			vbe->cur_y++;
		}

		if ( vbe->cur_y >= vbe->text_y - 1){
			for ( i = 0; i < vbe->text_y - 1; i++ ){
				// copy the next line of text in the framebuffer to the current line
				memcpy(
						vbe->framebuf + ( i      * vbe->fontfile->height) * vbe->pitch,
						vbe->framebuf + ((i + 1) * vbe->fontfile->height) * vbe->pitch,
						vbe->pitch * vbe->fontfile->height );

				// update the text buffer accordingly
				memcpy( textbuf + i * vbe->text_x, textbuf + (i + 1) * vbe->text_x, vbe->text_x );
			}

			vbe->cur_y = vbe->text_y - 2;
		}

		i = vbe->cur_x + vbe->cur_y * vbe->text_x;
		textbuf[i] = str[ret];
		vbe->cur_x++;

		//set_cursor( dev, vbe->cur_x, vbe->cur_y );
	}

	redraw_textbuf( vbe, startx, starty );
	//redraw_textbuf( vbe, 0, 0 );

	return ret;
}

int init( ){
	vbe_device_t *new_vbe;
	hal_device_t *new_haldev;
	vbe_mode_block_t *mode;
	tar_header_t *tarhead;

	kprintf( "[%s] Hello, initializin' yo vbe\n", provides );

	new_vbe = knew( vbe_device_t );

	new_vbe->mode = mode = (void *)bootheader->vbe_mode_info;
	new_vbe->info = (void *)bootheader->vbe_control_info;

	tarhead = initrd_get_file( main_initrd, "kernel/config/vbefont.psf" );

	if ( tarhead ){
		new_vbe->fontfile = (psf2_header_t *)(tarhead + 1);
	}

	kprintf( "[%s] info: 0x%x, mode: 0x%x, font: 0x%x\n",
			provides, new_vbe->info, new_vbe->mode, new_vbe->fontfile );

	if ( new_vbe->mode && new_vbe->info && tarhead ){
		kprintf( "[%s] X res: %d, Y res: %d, bpp: %d, physaddr: 0x%x\n",
				provides, mode->Xres, mode->Yres,
				mode->bpp, mode->physbase );

		kprintf( "[%s] fbconsole can hold %dx%d chars\n", __func__,
				mode->Xres / new_vbe->fontfile->width,
				mode->Yres / new_vbe->fontfile->height );

		new_vbe->text_x  = mode->Xres / new_vbe->fontfile->width;
		new_vbe->text_y  = mode->Yres / new_vbe->fontfile->height;
		new_vbe->textbuf = knew( uint8_t[new_vbe->text_y][new_vbe->text_x]);

		//new_vbe->framebuf = (void *)mode->physbase;
		new_vbe->framebuf = (void *)0xfb000000;
		new_vbe->x_res    = mode->Xres;
		new_vbe->y_res    = mode->Yres;
		new_vbe->bpp      = mode->bpp;
		new_vbe->pitch    = mode->pitch;

		unsigned pos = mode->physbase;
		unsigned end = mode->physbase + mode->Xres * 4 + mode->Yres * mode->pitch;
		unsigned fb_base = 0xfb000000;

		for ( ; pos < end; pos += PAGE_SIZE, fb_base += PAGE_SIZE ){
			map_r_page( get_current_page_dir( ), fb_base, pos, PAGE_WRITEABLE | PAGE_PRESENT );
		}
		flush_tlb( );

		clear_screen( new_vbe );

		new_haldev = knew( hal_device_t );
		new_haldev->block_size = 1;
		new_haldev->type = HAL_TYPE_VIDEO;
		new_haldev->write = vbe_console_hal_write;
		new_haldev->dev = new_vbe;
		new_haldev->name = strdup( "fbconsole" );
		hal_register_device( new_haldev );

		new_haldev = knew( hal_device_t );
		new_haldev->block_size = 1;
		new_haldev->type = HAL_TYPE_VIDEO;
		new_haldev->write = vbe_framebuf_hal_write;
		new_haldev->dev = new_vbe;
		new_haldev->name = strdup( "framebuffer" );
		hal_register_device( new_haldev );

	} else {
		kprintf( "[%s] No mode/control/font info available, aborting...\n", provides );
	}

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
