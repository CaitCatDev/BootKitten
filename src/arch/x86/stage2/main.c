#include <stdint.h>


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

__attribute__((noreturn)) void cmain() {
	volatile short *vga = (volatile short*)0xb8000;

	vga[0] = 0x1f00 | 'H';
	vga[1] = 0x1f00 | 'i';
	vga[3] = 0x1f00 | 'C';
	
	intx86(NULL, NULL, 0x10);
	vga[0] = 0x1f00 | 'Y'; 
	vga[1] = 0x1f00 | 'e';
	vga[2] = 0x1f00 | 's';
	while(1) {
		__asm__("cli");
		__asm__("hlt");
	}
}
