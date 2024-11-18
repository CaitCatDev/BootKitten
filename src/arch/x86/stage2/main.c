#include <stdint.h>
#include <x86int.h>
#include <kstdio.h>

#include <disk/gpt.h>
#include <disk/mbr.h>

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

typedef struct drive_parameters {
	uint16_t size;
	uint16_t flags;
	uint32_t cylinders;
	uint32_t heads;
	uint32_t sectors_per_track;
	uint64_t sectors;
	uint16_t sector_size;
	uint32_t edd_config;
	uint16_t dev_path_sig;
	uint8_t dev_path_leng;
	uint8_t reserved[3];
	char busname[4];
	char iftype[8];
	char interface_path[8];
	char device_path[8];
	uint8_t reserved1;
	uint8_t checksum;
}__attribute__((packed)) drive_parameters_t;


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

int memcmp(const void *m1, const void *m2, uint64_t size) {
	const uint8_t *m1u8 = m1;
	const uint8_t *m2u8 = m2;

	for(uint64_t i = 0; i < size; ++i) {
		if(m1u8[i] != m2u8[i]) {
			return m1u8 - m2u8;
		}
	}

	return 0;
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

void vga_mov_cursor(uint32_t x, uint32_t y) {
	uint16_t pos = y * 80 + x;

	out8(0x3d4, 0x0f);
	out8(0x3d5, pos);
	out8(0x3d4, 0x0e);
	out8(0x3d5, pos >> 8);
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

	vga_mov_cursor(x, y);
}

void vga_clear_screen(uint8_t color) {
	volatile short *vga = (volatile void*)0xb8000;
	for(uint32_t ly = 0; ly < ymax; ++ly) {
		for(uint32_t lx = 0; lx < xmax; ++lx) {
			vga[lx + ly * xmax] = (color << 8) + ' ';
		}
	}
	x = 0;
	y = 0;
	vga_mov_cursor(x, y);
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

void dump_registers_x86(const x86_regs_t *regs) {
	kprintf("EAX: 0x%x, EBX: 0x%x, ECX: 0x%x, EDX: 0x%x\n"
			"EBP: 0x%x, ESI: 0x%x, EDI: 0x%x, EFLAGS: 0x%x\n",
			regs->eax, regs->ebx, regs->ecx, regs->edx,
			regs->ebp, regs->esi, regs->edi, regs->eflags);
}

/*Determine if this disk looks like an MBR disk*/
/*This is harder than GPT as there is no direct magic bytes
 * we can just look at to determine what this disk is
 */
int is_mbr(mbr_t *mbr) {
	mbr_part_t part = { 0 }; /*Create a zeroed out part*/
	uint8_t no_parts = 1;
	if(!mbr) {
		return -1;
	}

	for(uint8_t i = 0; i < 4; ++i) {
		if(memcmp(&part, &mbr->parts[i], 16) != 0) {
			no_parts = 0;
		}
	}

	if(no_parts) {
		/*at this point it doesn't matter
		 *even if we are mbr we don't have any parts
		 */
		return -1;
	}

	if(mbr->parts[0].type == 0xee) {
		return -1; /*Protective MBR*/
	}

	/*We should maybe run more test but this works for early code*/
	return 0;
}

/*Determine if this disk looks like a GPT disk*/
int is_gpt(mbr_t *mbr, gpt_header_t *gpt) {
	if(!mbr || !gpt) {
		return -1;
	}

	if(mbr->parts[0].type != 0xee && memcmp("EFI PART", gpt->magic, 8) != 0) {
		return -1;
	}

	return 0;
}

__attribute__((noreturn)) void cmain(uint8_t disk) {
	volatile short *vga = (volatile short*)0xb8000;
	vga_clear_screen(0x1f);
	
#if defined DEBUG_TO_VGA
	kstdio_init(vga_put_ch); /*for when I don't have a serial out*/
#else
	kstdio_init(write_serial);
	if(init_serial(COM1) != 0) {
		kstdio_init(vga_put_ch); /*Fallback*/
		kprintf("Erorr Init Serial Using VGA instead\n");
	}
#endif

	kprintf("Stdio init done\n");
	x86_regs_t regs = { 0 };
	drive_parameters_t parameters = { 0 };
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

	/*Supports upto 16 disks*/
	for(uint32_t disk = 0x80; disk < 0x80 + *((uint8_t*)0x475); ++disk) {
		memset(&regs, 0, sizeof(regs));
		memset(&parameters, 0, sizeof(parameters));
		parameters.size = 0x42;
		parameters.flags = 0;
		regs.ds = (uint64_t)&parameters >> 4 & 0xf000;
		regs.esi = (uint64_t)&parameters;
		regs.edx = disk;
		regs.eax = 0x4800;

		intx86(&regs, &regs, 0x13);
		if(regs.eflags & 1) {
			kprintf("0x%x drive error: %d\n", disk, regs.eax);
			continue;
		} 
		
		kprintf("Disk 0x%x present sector Size: %d\n", disk, parameters.sector_size);
		if(parameters.size == 0x42) {
			kprintf("  - Bus: %s-%s\n", parameters.busname, parameters.iftype);
		}

		dap.size = 0x10;
		dap.lba = 0;
		dap.blocks = 2;
		dap.offset = 0x3000;

		regs.eax = 0x4200;
		regs.esi = (uint64_t)&dap;
		regs.edx = disk;
		intx86(&regs, &regs, 0x13);
		mbr_t *mbr = (void*)0x3000;
		gpt_header_t *gpt = (void*)0x3200;
		if(is_gpt(mbr, gpt) == 0) {
			kprintf("  - Disk is GPT Part Scheme\n");
			for(uint32_t i = 0; i < 128 / 4; ++i) {
				gpt_part_t empty = { 0 };
				gpt_part_t *parts = (void*)0x4000;
				regs.eax = 0x4200;
				dap.lba = 2 + i;
				dap.offset = 0x4000;
				dap.blocks = 1;
				intx86(&regs, &regs, 0x13);
				for(uint8_t j = 0; j < 4; ++j) {
					if(memcmp(&empty, &parts[j], sizeof(empty)) == 0) {
						continue;
					}
					kprintf("  - Part %d LBA Range: 0x%x-0x%x\n", j + (i * 4), parts[j].first_lba, parts[j].last_lba);
				}
			}

		} else if(is_mbr(mbr) == 0) {
			kprintf("  - Disk is MBR Part Scheme\n");
			mbr_part_t part = { 0 };
			for(uint8_t i = 0; i < 4; i++) {
				if(memcmp(&part, &mbr->parts[i], sizeof(part)) != 0) {
					kprintf("  - Part %d LBA Range: 0x%x-0x%x\n", i, mbr->parts[i].lba_start, mbr->parts[i].lba_start + mbr->parts[i].sectors);
				}
			}
		} else {
			kprintf("  - Disk is Unknown Part scheme\n");
		}

	}

	while(1) {
		_sleep(500);
	}
}
