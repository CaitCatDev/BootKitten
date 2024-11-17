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

typedef struct idt64_entry {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t ist;
	uint8_t attributes;
	uint16_t offset_med;
	uint32_t offset_hig;
	uint32_t zero;
} __attribute__((packed)) idt64_entry_t;

typedef struct idtr64 {
	uint16_t size;
	uint64_t offset;
} __attribute__((packed)) idtr64_t;

typedef struct dap {
	uint8_t size;
	uint8_t padding;
	uint16_t blocks;
	uint16_t offset;
	uint16_t segment;
	uint64_t lba;
} dap_t;


#define IDT_ENTRY_ATTR_PRESENT (1 << 7)
#define IDT_ENTRY_ATTR_INT_GATE (0xe)
#define IDT_ENTRY_ATTR_TRAP_GATE (0xf)
#define NULL (void*)0

__attribute__((aligned(0x10)))
static idt64_entry_t idt[256];


extern void isr_no_err();
extern void _sleep(uint64_t ms);

static uint16_t *ivt = (void*)0x0020;

extern void catch_irq08();
extern uint32_t originalirq8;
extern void _boot_multiboot2();

void idt_set_entry(uint8_t vector, void *function, uint8_t gate_type) {
	idt[vector].selector = 0x28;
	idt[vector].attributes = gate_type;
	idt[vector].offset_low = (uint64_t)function & 0xffff;
	idt[vector].offset_med = ((uint64_t)function >> 16) & 0xffff;
	idt[vector].offset_hig = ((uint64_t)function >> 32) & 0xffffffff;
}

void *memset(void *dst, int val, uint64_t n) {
	uint8_t *dstu8 = dst;

	for(uint64_t i = 0; i < n; i++) {
		dstu8[i] = val;
	}

	return dst;
}

void *memcpy(void *dst, const void *src, uint64_t n) {
	uint8_t *u8dst = dst;
	const uint8_t *u8src = src;

	for(uint64_t i = 0; i < n; i++) {
		u8dst[i] = u8src[i];
	}

	return dst;
}

__attribute__((noreturn)) void cmain() {
	volatile short *vga = (volatile short*)0xb8000;
	x86_regs_t regs = { 0 };
	e820_mmap_t map = { 0 };
	dap_t dap = { 0 };

	memset(idt, 0, sizeof(idt));

	for(uint8_t v = 0; v < 32; v++) {
		idt_set_entry(v, &isr_no_err, 0x8e);
	}
	originalirq8 = 0;
	originalirq8 |= ivt[1] << 16;
	originalirq8 |= ivt[0];
	ivt[1] = (((uint64_t)catch_irq08) >> 4) & 0xf000;
	ivt[0] = (((uint64_t)catch_irq08)) & 0xffff;

	idtr64_t idtr;
	idtr.size = sizeof(idt) - 1;
	idtr.offset = (uint64_t)idt;
	
	/*CLI is set so we only trigger this on actual exceptions*/
	__asm__ volatile("lidt %0" : : "m" (idtr));
	
	vga[0] = 0x1f00 | 'B';
	vga[1] = 0x1f00 | 'o';	
	vga[2] = 0x1f00 | 'o';
	vga[3] = 0x1f00 | 't';
	vga[4] = 0x1f00 | 'i';	
	vga[5] = 0x1f00 | 'n';
	vga[6] = 0x1f00 | 'g';


	for(uint64_t i = 3; i > 0; i--) {
		vga[8] = 0x1f00 | i + 0x30;
		_sleep(1000);
	}

	vga[8] = ' ';

	dap.lba = 7;
	dap.size = 0x10;
	dap.blocks = (36112 / 512) + 1;
	dap.offset = 0x1000;
	dap.segment = 0x5000;

	regs.eax = 0x4200;
	regs.edx = 0x80;
	regs.ds = (((uint64_t)&dap) >> 4) & 0xf000;
	regs.esi = (((uint64_t)&dap) & 0xffff);

	intx86(&regs, &regs, 0x13);
	/*This only works cause of ungodly hard coding but yea
	 *It is based on my kernel image and this just an initial proof of 
	 *concept
	 *TODO: Load Kernel from filesystem more inteligently I.E. finding it
	 *on the disk rather than just assuming it's at sector 6
	 *TODO: Load the Kernel elf correctly. I.E. actually parse the headers
	 *TODO: support kernels bigger than 64KB
	 *TODO: Also check that memory at that address is even free(it normally should be)
	 */
	/*Load Text*/
	memcpy((void*)0x100000, (const void *)0x52000, 0x5000);
	/*Load Rodata*/
	memcpy((void*)0x105000, (const void *)0x57000, 0x16b8);
	memcpy((void*)0x107000, (const void *)0x59000, 0x51);

	_boot_multiboot2();

	while(1) {
		_sleep(1000);
	}
}
