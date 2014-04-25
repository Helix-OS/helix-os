#include <vga/vga.h>
#include <base/kstd.h>
#include <base/hal.h>
#include <base/logger.h>
#include <base/string.h>

char *depends[] = { "base", 0 };
char *provides = "vga";

static int vga_hal_write( struct hal_device *dev, void *buf, unsigned count, unsigned offset );

/* Offset is ignored in this function, since it's handled by the driver */
static int vga_hal_write( struct hal_device *dev, void *buf, unsigned count, unsigned offset ){
	vga_device_t *vga = dev->dev;
	vga_char_t *buffer = vga->buffer;
	char *str = buf;
	int ret = 0;
	int i;

	for ( ret = 0; ret < count; ret++ ){
		switch ( str[ret] ){
			case '\n':
				vga->cur_y++;
			case '\r':
				vga->cur_x = 0;
				continue;
			default:
				break;
		}

		if ( vga->cur_x == vga->max_x ){
			vga->cur_x = 0;
			vga->cur_y++;
		}

		if ( vga->cur_y >= vga->max_y ){
			for ( i = 0; i < vga->max_y; i++ )
				memcpy( buffer + i * vga->max_x, buffer + (i + 1) * vga->max_x,
						sizeof( vga_char_t[vga->max_x] ));

			vga->cur_y = vga->max_y - 1;
		}

		i = vga->cur_x + vga->cur_y * vga->max_x;

		buffer[i].color = VGA_COLOR_GRAY;
		buffer[i].letter = str[ret];

		vga->cur_x++;
	}

	return ret;
}

int init( ){
	vga_device_t *new_vga;
	hal_device_t *new_haldev;
	vga_char_t *charbuf;
	int i;

	kprintf( "[%s] Hello, initializin' yo vga\n", provides );

	new_vga = knew( vga_device_t );
	charbuf = new_vga->buffer = (vga_char_t *)0xb8000;
	new_vga->max_x = 80;
	new_vga->max_y = 25;

	new_haldev = knew( hal_device_t );
	new_haldev->block_size = 1;
	new_haldev->type = HAL_TYPE_VIDEO;
	new_haldev->write = vga_hal_write;
	new_haldev->dev = new_vga;
	hal_register_device( new_haldev );

	for ( i = 0; i < 80 * 25; i++ ){
		charbuf[i].color = 0;
		charbuf[i].letter = ' ';
	}

	char *teststr = "Cool, VGA text mode is working.\n";
	vga_hal_write( new_haldev, teststr, strlen( teststr ), 0 );

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
