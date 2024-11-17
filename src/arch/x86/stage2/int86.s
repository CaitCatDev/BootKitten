.code64
.text
.global intx86
intx86:
	push %rbp
	mov %rsp,%rbp /*Set up Stack Frame*/

	pushq %rbx
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	movb %dl,int_no

	cli

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
	sti
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
	mov %esi,_rout

	sgdt _saved_gdt
	sidt _saved_idt
	
	/*back to BIOS IVT*/
	lidt _real_idt

	mov %cr0,%eax
	and $0xff,%eax
	mov %eax,%cr0

	mov $intx86_16,%ebx
	push $0x8
	and $0xffff,%ebx /*cast off upper bits to avoid overflow*/
	push %ebx
	retf


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

	mov $intx86_real,%ebx
	shr $4,%ebx
	and $0xf000,%ebx
	push %bx
	mov $intx86_real,%ebx
	and $0xffff,%ebx /*cast off upper bits to avoid overflow*/
	push %bx
	retf


intx86_real:
	mov %cs,%ax #TODO: when segement changes this needs to aswell
	mov %ax,%ds
	mov %ax,%es
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
	
	/*Restore SS*/
	bswap %esp
	shl $4,%sp
	mov %sp,%ss
	shr $4,%sp
	bswap %esp
	
	sti
	hlt

	/*https://www.felixcloutier.com/x86/intn:into:int3:int1*/
	.byte 0xcd
	int_no: .byte 0x00 /*Overwrite at runtime*/

	cli
	/*Hmmm hecc. so we need to get the stack setup to output
	 * registers but we don't wanna overwrite registers 
	 */
	mov $_rout,%esp
	shr $4,%esp
	and $0xf000,%sp
	mov %sp,%ss
	mov $_rout,%esp
	pop %esp

	bswap %esp
	shl $4,%sp
	mov %sp,%ss
	shr $4,%sp
	bswap %esp
	add $4*8+2*4,%sp

	push %fs
	push %gs
	push %es
	push %ds

	pushfl
	push %ebp
	
	push %esi
	push %edi

	push %edx
	push %ebx
	push %ecx
	push %eax
	mov $_saved_stack,%esp
	shr $4,%esp
	and $0xf000,%sp
	mov %sp,%ss
	mov $_saved_stack,%esp
	pop %esp
	/*Restore SS*/
	bswap %esp
	shl $4,%sp
	mov %sp,%ss
	shr $4,%sp
	bswap %esp
	
	/*Reload original IVT and GDT*/
	mov $_saved_gdt,%eax
	mov %ax,%bx
	shr $4,%eax
	and $0xf000,%ax
	mov %ax,%ds

	lgdt (%bx)	

	mov $_saved_idt,%eax
	mov %ax,%bx
	shr $4,%eax
	and $0xf000,%ax
	mov %ax,%ds
	
	lidt (%bx)

	mov %cr0,%eax
	or $1,%eax
	mov %eax,%cr0

	jmpl $0x18,$intx86_32_ret

.bss
_rout:
	.long 0x0000

_saved_stack:
	.long 0x0000

_saved_gdt:
	.word 0x0000
	.long 0x0000

_saved_idt:
	.word 0x0000
	.long 0x0000

.global _saved_idt
.global _saved_gdt

.data
.global _real_idt
_real_idt:
	.word 0x03ff
	.long 0x0000
