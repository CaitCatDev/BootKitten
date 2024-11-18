#include <stdint.h>
#include <x86int.h>
#include <kstdio.h>

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

static inline void out8(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

static inline uint8_t in8(uint16_t port) {
    uint8_t data;
    __asm__ volatile("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void out16(uint16_t port, uint16_t data) {
    __asm__ volatile("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

static inline uint16_t in16(uint16_t port) {
    uint16_t data;
    __asm__ volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void out32(uint16_t port, uint32_t data) {
    __asm__ volatile("outl %0, %w1" : : "a" (data), "Nd" (port));
}

static inline uint32_t in32(uint16_t port) {
    uint32_t data;
    __asm__ volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
    return data;
}


#define COM1 0x3f8
int is_transmit_empty(uint16_t port) {
	return in8(port + 5) & 0x20;
}

void write_serial(uint8_t a) {
	while (is_transmit_empty(COM1) == 0);
	
	if(a == '\n') {
		out8(COM1, '\r');
		while (is_transmit_empty(COM1) == 0);
	}
	out8(COM1,a);
}

static uint32_t x = 0;
static uint32_t y = 0;
static uint32_t xmax = 80;
static uint32_t ymax = 25;

void vga_clear_line(uint32_t line, uint8_t color) {
	volatile short *vga = (volatile void*)0xb8000;
	for(uint32_t lx = 0; lx < xmax; ++lx) {
		vga[lx + line * xmax] = (color << 8) + ' ';
	}
}

void vga_put_ch(uint8_t ch) {
	volatile char *vga;

	/*Scrolls the screen*/
	if(y >= ymax) {
		for(y = 0; y < ymax-1; ++y) {
			memcpy((void*)((uint64_t)0xb8000 + (y * 160)), (const void*)((uint64_t)0xb8000 + ((y+1) * 160)), 160);
		}
		y = ymax - 1;
		x = 0;
		vga_clear_line(y, 0x1f);
	}

	vga = (volatile void*)0xb8000 + x * 2 + y * 160;

	if(ch == '\n') {
		y++;
		x=0;
	} else {
		*vga = ch;
		x++;
	}
}

void vga_clear_screen(uint8_t color) {
	volatile short *vga = (volatile void*)0xb8000;
	for(uint32_t ly = 0; ly < ymax; ++ly) {
		for(uint32_t lx = 0; lx < xmax; ++lx) {
			vga[lx + ly * xmax] = (color << 8) + ' ';
		}
	}
}

int init_serial(uint16_t port) {
	out8(port + 1, 0x00);
	out8(port + 3, 0x80);
	
	/*Set timer*/
	out8(port + 0, 0x03);
	out8(port + 1, 0x00);

	out8(port + 3, 3);
	
	/*FIFO*/
	out8(port + 2, 0xc7);
	out8(port + 4, 0x0b);
	out8(port + 4, 0x1e);

	/*Loopback*/
	out8(port, 0xca);

	if(in8(port) != 0xca) {
		return -1;
	}
	
	out8(port + 4, 0xf);
	return 0;
}

__attribute__((noreturn)) void cmain(uint8_t disk) {
	volatile short *vga = (volatile short*)0xb8000;
	vga_clear_screen(0x1f);
	
#if defined DEBUG_TO_VGA
	kstdio_init(vga_put_ch); /*for when I don't have a serial out*/
#else
	kstdio_init(write_serial);
	if(init_serial(COM1) == 0) {
		kstdio_init(vga_put_ch); /*Fallback*/
		kprintf("Erorr Init Serial Using VGA instead\n");
	}
#endif

	kprintf("Stdio init done\n");
	x86_regs_t regs = { 0 };
	e820_mmap_t map = { 0 };
	dap_t dap = { 0 };

	memset(idt, 0, sizeof(idt));

	for(uint8_t v = 0; v < 32; v++) {
		/*TODO: Actually build and exception ISR table*/
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
	

	for(uint64_t i = 3; i > 0; i--) {
		kprintf("Booting... %d\n", i);		
		_sleep(1000);
	}

	while(1) {
		_sleep(500);
}
}
