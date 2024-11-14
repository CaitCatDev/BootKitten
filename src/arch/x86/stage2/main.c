#include <stdint.h>

#include <x86int.h>

typedef struct e820_mmap {
	uint32_t base_low;
	uint32_t base_hi;
	uint32_t length_lo;
	uint32_t length_hi;
	uint32_t type;
	uint32_t acpi;
} e820_mmap_t;

#define NULL (void*)0

void *memset(void *dst, int val, uint64_t n) {
	uint8_t *dstu8 = dst;

	for(uint64_t i = 0; i < n; i++) {
		dstu8[i] = val;
	}

	return dst;
}

__attribute__((noreturn)) void cmain() {
	volatile short *vga = (volatile short*)0xb8000;
	x86_regs_t regs = { 0 };
	e820_mmap_t map = { 0 };

	regs.eax = 0xe820;
	regs.edx = 0x534D4150;
	regs.ebx = 0;
	regs.edi = ((uint64_t)&map) & 0xffffffff;
	regs.ecx = 0x24;
	regs.es = (((uint64_t)&map) >> 4) & 0xf000;

	vga[0] = 0x1f00 | 'H';
	vga[1] = 0x1f00 | 'i';
	vga[3] = 0x1f00 | 'C';
	
	intx86(&regs, &regs, 0x15);

	if(regs.eax == 'SMAP') {	
		vga[0] = 0x1f00 | 'Y';
		vga[1] = 0x1f00 | 'e';
		vga[2] = 0x1f00 | 's';

	}
	while(1) {
		__asm__("cli");
		__asm__("hlt");
	}
}
