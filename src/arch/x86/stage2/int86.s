.code64
.text
.global intx86
	/* TODO Take registers IN(Interrupt Arguements)
	 * TODO Store Registers OUT(Interrupt Results)
	 * TODO Encode the INT instruction at runtime .byte 0xcd 0x00 then overwrite 0x00
	 * TODO Save and Restore IDT, GDT and page tables possibly.
	 * TODO When we use our Own IDT we need to swap back to the BIOS's IDT.
	 * TODO Change segment when the Linker script is updated as I plan to change where this code executes from.
   */
intx86:
	pushq $0x18
	pushq $intx86_32
	lretq

intx86_ret:
	mov $0x30,%ax
	
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs

	ret

.code32

intx86_32:
	mov $0x20,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs
	
	mov %cr0,%eax
	and $0xff,%eax
	mov %eax,%cr0

	jmp $0x08,$intx86_16

intx86_32_ret:
	mov $0x20,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs

	mov %cr0,%eax
	mov $0x80000011,%eax
	mov %eax,%cr0

	jmp $0x28,$intx86_ret


.code16
intx86_16:
	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs


	mov %cr0,%eax
	and $0xfe,%eax
	mov %eax,%cr0

	jmp $0x0000,$intx86_real


intx86_real:
	xor %ax,%ax #TODO: when segement changes this needs to aswell
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs

	sti

	mov $1,%ax
	int $0x10

	cli
	mov %cr0,%eax
	or $1,%eax
	mov %eax,%cr0

	jmp $0x18,$intx86_32_ret
