#ifndef _helix_vga_module_h
#define _helix_vga_module_h
#include <base/stdint.h>

enum {
	VGA_MODE_NULL,
	VGA_MODE_TEXTBUF,
	VGA_MODE_FRAMEBUF,
	VGA_MODE_VESA, 		// I know it's not part of vga, just
				// easier to handle the code here
};

enum {
	VGA_COLOR_BLACK,
	VGA_COLOR_BLUE,
	VGA_COLOR_GREEN,
	VGA_COLOR_CYAN,
	VGA_COLOR_RED,
	VGA_COLOR_MAROON,
	VGA_COLOR_BROWN,
	VGA_COLOR_GRAY,
};

enum {
	VGA_COLOR_FOREGROUND = 0,
	VGA_COLOR_BACKGROUND = 4,
	VGA_COLOR_BOLD       = 8,
};

typedef struct vga_device {
	void 		*buffer;
	unsigned 	mode;
	unsigned 	*baseaddr;
	unsigned 	max_x;
	unsigned 	max_y;
	unsigned 	bpp;

	unsigned 	cur_x;
	unsigned 	cur_y;
} vga_device_t;

typedef struct vga_char {
	uint8_t letter;
	uint8_t color;
} __attribute__((packed)) vga_char_t;

#endif
