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

static int vbe_hal_write( struct hal_device *dev, void *buf, unsigned count, unsigned offset );

/* Offset is ignored in this function, since it's handled by the driver */
static int vbe_hal_write( struct hal_device *dev, void *buf, unsigned count, unsigned offset ){
	//vbe_device_t *vbe = dev->dev;
	//vbe_char_t *buffer = vbe->buffer;
    /*
	char *str = buf;
	int ret = 0;
	int i;

	for ( ret = 0; ret < count; ret++ ){
		switch ( str[ret] ){
			case '\n':
				vbe->cur_y++;
			case '\r':
				vbe->cur_x = 0;
				continue;
			case '\b':
				vbe->cur_x--;

				i = vbe->cur_x + vbe->cur_y * vbe->max_x;
				buffer[i].letter = 0;

				set_cursor( dev, vbe->cur_x, vbe->cur_y );
				continue;
			default:
				break;
		}

		if ( vbe->cur_x == vbe->max_x ){
			vbe->cur_x = 0;
			vbe->cur_y++;
		}

		if ( vbe->cur_y >= vbe->max_y ){
			for ( i = 0; i < vbe->max_y; i++ )
				memcpy( buffer + i * vbe->max_x, buffer + (i + 1) * vbe->max_x,
						sizeof( vbe_char_t[vbe->max_x] ));

			vbe->cur_y = vbe->max_y - 1;
		}

		i = vbe->cur_x + vbe->cur_y * vbe->max_x;

		buffer[i].color = vbe_COLOR_GRAY;
		buffer[i].letter = str[ret];

		vbe->cur_x++;

		buffer[i+1].color = vbe_COLOR_GRAY;
		set_cursor( dev, vbe->cur_x, vbe->cur_y );
	}

	return ret;
    */
    return -1;
}

void set_pixel( vbe_device_t *dev, unsigned x, unsigned y, uint32_t val ){
    if ( x < dev->x_res && y < dev->y_res ){
        uint32_t *place = (uint32_t *)(dev->framebuf + y * dev->pitch + x * 4);
        *place = val;
    }
}

int init( ){
	vbe_device_t *new_vbe;
	hal_device_t *new_haldev;

	kprintf( "[%s] Hello, initializin' yo vbe\n", provides );

	new_vbe = knew( vbe_device_t );

    new_vbe->mode = (void *)bootheader->vbe_mode_info;
    new_vbe->info = (void *)bootheader->vbe_control_info;

    kprintf( "[%s] info: 0x%x, mode: 0x%x\n", provides, new_vbe->info, new_vbe->mode );

    if ( new_vbe->mode && new_vbe->info ){
        kprintf( "[%s] X res: %d, Y res: %d, bpp: %d, physaddr: 0x%x\n",
                provides, new_vbe->mode->Xres, new_vbe->mode->Yres,
                new_vbe->mode->bpp, new_vbe->mode->physbase );

        new_vbe->framebuf = (void *)new_vbe->mode->physbase;
        new_vbe->x_res = new_vbe->mode->Xres;
        new_vbe->y_res = new_vbe->mode->Yres;
        new_vbe->bpp = new_vbe->mode->bpp;
        new_vbe->pitch = new_vbe->mode->pitch;

        unsigned pos = new_vbe->mode->physbase;
        unsigned end = new_vbe->mode->physbase + new_vbe->mode->Xres * 4
            + new_vbe->mode->Yres * new_vbe->mode->pitch;

        for ( ; pos < end; pos += PAGE_SIZE ){
            map_r_page( get_current_page_dir( ), pos, pos, PAGE_WRITEABLE | PAGE_PRESENT );
        }
        flush_tlb( );

        //memset( (void *)0xa0000, 0xff, 1000 );
        unsigned x, y;
        for ( x = 0, y = 0; y < new_vbe->mode->Yres - 1; y++, x++ ){
            set_pixel( new_vbe, x, y, 0x00ffff00 );
        }

        for ( x = 0, y = new_vbe->mode->Yres - 1; y > 0; y--, x++ ){
            set_pixel( new_vbe, x, y, 0x00ff00ff );
        }

        new_haldev = knew( hal_device_t );
        new_haldev->block_size = 1;
        new_haldev->type = HAL_TYPE_VIDEO;
        new_haldev->write = vbe_hal_write;
        new_haldev->dev = new_vbe;
        //new_haldev->name = strdup( "framebuf" );
        new_haldev->name = strdup( "fbconsole" );
        hal_register_device( new_haldev );

    } else {
        kprintf( "[%s] No mode/control info available, aborting...\n", provides );
    }
    /*
	for ( i = 0; i < 80 * 25; i++ ){
		charbuf[i].color = 0;
		charbuf[i].letter = ' ';
	}
    */

	//char *teststr = "Cool, vbe text mode is working.\n";
	//vbe_hal_write( new_haldev, teststr, strlen( teststr ), 0 );

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
