ENTRY (loader)

SECTIONS {
	/* . = 0x00100000; */
	. = 0xc0100000;

	.__mbHeader : AT(ADDR(.__mbHeader) - 0xc0000000) {
		*(.__mbHeader)
	}

	.text ALIGN (0x1000) : AT(ADDR(.text) - 0xc0000000) {
		*(.text)
	}

	.rodata ALIGN (0x1000) : AT(ADDR(.rodata) - 0xc0000000) {
		*(.rodata*)
	}

	.data ALIGN (0x1000) : AT(ADDR(.data) - 0xc0000000) {
		*(.data)
	}

	.bss ALIGN (0x1000) : AT(ADDR(.bss) - 0xc0000000) {
		sbss = .;
		*(COMMON)
		*(.bss)
		ebss = .;
	}

	end = .; _end = .; __end = .;
}
