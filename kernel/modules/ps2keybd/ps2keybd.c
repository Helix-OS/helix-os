#include <base/logger.h>
#include <base/arch/i586/isr.h>
#include <base/datastructs/pipe.h>
#include <base/string.h>
#include <base/stdio.h>
#include <base/hal.h>
#include <base/kstd.h>
#include <base/mem/alloc.h>
#include <base/tasking/task.h>

char *depends[] = { "base", "hal", 0 };
char *provides = "ps2kbd";

static int kbd_hal_read( struct hal_device *dev, void *buf, unsigned count, unsigned offset );

enum {
	KEYMETA_NULL,
	KEYMETA_CTRL 		= 1,
	KEYMETA_SHIFT 		= 2,
	KEYMETA_ALT 		= 4,
	KEYMETA_CAPSLOCK 	= 8,
	KEYMETA_NUMLOCK 	= 16,
	KEYMETA_SCROLL_LOCK 	= 32,
	KEYMETA_LAST 		= 32,
};

pipe_t *keyboard_pipe = 0;
unsigned char metastatus = 0;
char **keymap;
semaphore_t keybd_sem;

/* Modifier/meta keys are encoded as integers, and checked for in the handler.
 * This is fine because the module (or any module for that matter) will not be loaded
 * at page 0.
 */
static char *us_keymap_lower[] = {
	/* standard alphanumerics */
	"", 	"\e[",
	"1", 	"2", 	"3", 	"4",	"5",	"6",	"7",	"8",	"9",	"0",
	"-",	"=",	"\b",	"\t",
	"q",	"w",	"e",	"r",	"t",	"y",	"u",	"i",	"o",	"p",	"[",	"]",	"\n",
	(char *)KEYMETA_CTRL,
	"a",	"s",	"d",	"f",	"g",	"h",	"j",	"k",	"l",	";",	"'",	"`",
	(char *)KEYMETA_SHIFT, "\\",
	"z",	"x",	"c",	"v",	"b",	"n",	"m",	",",	".",	"/",
	(char *)KEYMETA_SHIFT,	"*",	(char *)KEYMETA_ALT,	" ",
	(char *)KEYMETA_CAPSLOCK, 

	/* Function keys f1 - f10 */
	"\e[[11~", "\e[[12~", "\e[[13~", "\e[[14~", "\e[[15~", "\e[[17~",
	"\e[[18~", "\e[[19~", "\e[[20~", "\e[[21~",

	/* meta keys */
	(char *)KEYMETA_NUMLOCK,
	(char *)KEYMETA_SCROLL_LOCK, 

	/* Keypad keys */
	     "7", "8", "9",
	"-", "4", "5", "6",
	"+", "1", "2", "3", 
	"0", ".",

	/* Unassigned keys */
	"", "", "",
	
	/* Function keys f11 - f12 */
	"\e[[23~", "\e[[24~", 

	/* More unassigned keys */
	"", "", "",
};

static char *us_keymap_upper[] = {
	/* standard alphanumerics */
	"", 	"\e[",
	"!", 	"@", 	"#", 	"$",	"%",	"^",	"&",	"*",	"(",	")",
	"_",	"+",	"\b",	"\t",
	"Q",	"W",	"E",	"R",	"T",	"Y",	"U",	"I",	"O",	"P",	"{",	"}",	"\n",
	(char *)KEYMETA_CTRL,
	"A",	"S",	"D",	"F",	"G",	"H",	"J",	"K",	"L",	":",	"\"",	"~",
	(char *)KEYMETA_SHIFT, "|",
	"Z",	"X",	"C",	"V",	"B",	"N",	"M",	"<",	">",	"?",
	(char *)KEYMETA_SHIFT,	"*",	(char *)KEYMETA_ALT,	" ",
	(char *)KEYMETA_CAPSLOCK, 

	/* Function keys f1 - f10 */
	"\e[[11~", "\e[[12~", "\e[[13~", "\e[[14~", "\e[[15~", "\e[[17~",
	"\e[[18~", "\e[[19~", "\e[[20~", "\e[[21~",

	/* meta keys */
	(char *)KEYMETA_NUMLOCK,
	(char *)KEYMETA_SCROLL_LOCK, 

	/* Keypad keys */
	     "7", "8", "9",
	"-", "4", "5", "6",
	"+", "1", "2", "3", 
	"0", ".",

	/* Unassigned keys */
	"", "", "",
	
	/* Function keys f11 - f12 */
	"\e[[23~", "\e[[24~", 

	/* More unassigned keys */
	"", "", "",
};

static int kbd_hal_read( struct hal_device *dev, void *buf, unsigned count, unsigned offset ){
	int ret = 0;
	pipeline_t *line = dev->dev;

	
	while( 1 ){
		enter_semaphore( &keybd_sem );
		ret = pipeline_read( line, buf, count );
		leave_semaphore( &keybd_sem );

		if ( !ret )
			usleep( 10 ); // just block if there's nothing to read
		else
			break;
	}


	return ret;
}

static void keyboard_handler( registers_t *regs ){
	unsigned char scancode;
	char *buf;

	scancode = inb( 0x60 );

	if ( scancode & 0x80 ){
		buf = keymap[ scancode - 0x80 ];
		if ((unsigned)buf <= KEYMETA_LAST ){
			metastatus &= ~(unsigned)buf;

			if ((( metastatus & KEYMETA_SHIFT ) ^ ( metastatus & KEYMETA_CAPSLOCK )) == 0 )
				keymap = us_keymap_lower;
		}

	} else {
		buf = keymap[ scancode ];

		if ((unsigned)buf <= KEYMETA_LAST ){
			metastatus |= (unsigned)buf;

			if (( metastatus & KEYMETA_SHIFT ) ^ ( metastatus & KEYMETA_CAPSLOCK ))
				keymap = us_keymap_upper;

		} else {
			if ( buf ){
				keybd_sem = 0;

				char *ctrl_seq = "\e";
				if ( metastatus & KEYMETA_CTRL )
					pipe_write( keyboard_pipe, ctrl_seq, (unsigned)strlen( ctrl_seq ));

				pipe_write( keyboard_pipe, buf, (unsigned)strlen( buf ));

				keybd_sem = 1;
				//kprintf( "[%s] Got buffer \"%s\", meta keys: 0x%x\n", __func__, buf, metastatus );
			}
		}
	}

	return;
}

int init( ){
	hal_device_t *new_dev;
	
	new_dev = knew( hal_device_t );
	keyboard_pipe = pipe_create( 1, 128, PIPE_FLAG_NULL ); 
	keymap = us_keymap_lower;

	new_dev->type = HAL_TYPE_PERIPHERAL;
	new_dev->subtype = HAL_PERIPHERAL_KEYBOARD;
	new_dev->dev = keyboard_pipe->bufs[0];
	new_dev->read = kbd_hal_read;
	new_dev->block_size = 1;
	new_dev->name = strdup( "keyboard" );

	init_semaphore( &keybd_sem, 1 );

	register_interrupt_handler( IRQ1, keyboard_handler );
	hal_register_device( new_dev );

	kprintf( "[%s] Hello world!\n", provides );
	kprintf( "[%s] This is module \"%s\", and I'm in yo kernelz\n",
			provides, provides );
	return 0;
}

void remove( ){
	unregister_interrupt_handler( IRQ1 );
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
