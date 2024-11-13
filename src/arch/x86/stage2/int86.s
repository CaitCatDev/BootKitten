.code64
.text
.global intx86
	/*
	 * TODO Store Registers OUT(Interrupt Results)
	 * TODO Save and Restore IDT, GDT and page tables possibly.
	 * TODO When we use our Own IDT we need to swap back to the BIOS's IDT.
	 * TODO Change segment when the Linker script is updated as I plan to change where this code executes from.
   */
intx86:
	push %rbp
	mov %rsp,%rbp /*Set up Stack Frame*/

	pushq %rbx
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	movb %dl,int_no

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

	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbx

	pop %rbp

	ret

.code32

intx86_32:
	mov $0x20,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs
	

	mov %esp,_saved_stack /*Save the REAL stack*/

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

	/*Stack(%ss:%sp) now points to our code*/
	mov %di,%sp
	shr $4,%edi
	and $0xf000,%di
	mov %di,%ss
	
	pop %eax
	pop %ecx
	pop %ebx
	pop %edx

	pop %edi
	pop %esi

	popfl
	pop %ebp
	
	pop %ds
	pop %es
	pop %gs
	pop %fs

	/*STACK SHOULD BE RESTORED NOW*/
	mov $_saved_stack,%esp
	shr $4,%esp
	and $0xf000,%sp
	mov $_saved_stack,%esp
	pop %esp
	
	sti
	/*https://www.felixcloutier.com/x86/intn:into:int3:int1*/
	.byte 0xcd
	int_no: .byte 0x00 /*Overwrite at runtime*/

	cli
	mov %cr0,%eax
	or $1,%eax
	mov %eax,%cr0

	jmp $0x18,$intx86_32_ret


_saved_stack:
	.long 0xCAFE
