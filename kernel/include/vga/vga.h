#ifndef _helix_vga_module_h
#define _helix_vga_module_h

enum {
	VGA_MODE_NULL,
	VGA_MODE_TEXTBUF,
	VGA_MODE_FRAMEBUF,
	VGA_MODE_VESA, 		// I know it's not part of vga, just
				// easier to handle the code here
}

typedef struct vga_device {
	unsigned 	mode;
	unsigned 	*baseaddr;
	unsigned 	max_x;
	unsigned 	max_y;
	unsigned 	bpp;

	unsigned 	cur_x;
	unsigned 	cur_y;
} vga_device_t;

#endif
