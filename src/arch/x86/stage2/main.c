#include <stdint.h>

/* I don't think any interrupts use the stack this will break if they do
 * but will cross that bridge when I come to it
 * **EDIT** Rather there are no ints that'll trash the stack they will use it
 */
typedef struct x86_regs {
	/*General Purpose Regs*/
	uint32_t eax, ecx, ebx, edx;
	
	/*Index Regs*/
	uint32_t edi, esi;

	/*Flags and base*/
	uint32_t eflags, ebp;

	/*Segment Registers*/
	uint16_t ds, es, gs, fs;
} x86_regs_t;

void intx86(x86_regs_t *in, x86_regs_t *out, uint8_t no);

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

	regs.eax = 3;

	vga[0] = 0x1f00 | 'H';
	vga[1] = 0x1f00 | 'i';
	vga[3] = 0x1f00 | 'C';
	
	intx86(&regs, NULL, 0x10);
	vga[0] = 0x1f00 | 'Y'; 
	vga[1] = 0x1f00 | 'e';
	vga[2] = 0x1f00 | 's';
	while(1) {
		__asm__("cli");
		__asm__("hlt");
	}
}
