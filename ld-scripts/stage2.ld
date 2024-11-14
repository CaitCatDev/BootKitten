ENTRY(_start)

SECTIONS {
	. = 0x10000;
	.text : {
		*(.text)
	}
	
	.data : {
		*(.data)
	}

	.rodata : {
		*(.rodata)
	}

	.bss : ALIGN(64K) {
		*(.bss)
	}
}
