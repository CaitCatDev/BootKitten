#pragma once

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
