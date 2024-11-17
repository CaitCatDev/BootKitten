.code64
.text
.global isr_no_err

.macro isr_save_regs
	push %rax
	push %rbx
	push %rcx
	push %rdx

	push %rsi
	push %rdi

	push %r8
	push %r9
	push %r10
	push %r11
	
	push %r12
	push %r13
	push %r14
	push %r15

	pushfq
	push %rbp
.endm

.macro isr_restore_regs
	pop %rbp
	popfq

	pop %r15
	pop %r14
	pop %r13
	pop %r12

	pop %r11
	pop %r10
	pop %r9
	pop %r8

	pop %rdi
	pop %rsi

	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax
.endm

isr_no_err:
	pushq 0x01 /*Dummy Vector*/

exception:
	isr_save_regs

_never_leave:
	jmp _never_leave
	isr_restore_regs
	
	add $16,%rsp
	iretq

.global _sleep
_sleep:
	/*atm we on BIOS clock so 54.8xxx MS per tick.
	 *But we arent super concerned about being accurate rn
	 *Just a rough test
	 *TODO Make this code more accurate i.e. count fractions of MS
	 */
	push %rbp
	mov %rsp,%rbp /*Set up Stack Frame*/

	pushq %rbx
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	
	xor %rdx,%rdx
	mov $55,%rcx
	mov %rdi,%rax
	div %rcx

	mov %eax,_ticks;

	pushq $0x18
	pushq $_sleep_32
	lretq

_sleep_ret:
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
	retq


.code32
_sleep_32:
	mov $0x20,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs
	
	sgdt _saved_gdt
	sidt _saved_idt
	
	/*back to BIOS IVT*/
	lidt _real_idt

	mov %cr0,%eax
	and $0xff,%eax
	mov %eax,%cr0

	mov $sleep_16,%ebx
	push $0x8
	and $0xffff,%ebx /*cast off upper bits to avoid overflow*/
	push %ebx
	retf


_sleep_32_ret:
	mov $0x20,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs

	mov %cr0,%eax
	mov $0x80000011,%eax
	mov %eax,%cr0

	jmp $0x28,$_sleep_ret


.code16
sleep_16:
	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs

	mov %cr0,%eax
	and $0xfe,%eax
	mov %eax,%cr0

	mov $sleep_real,%ebx
	shr $4,%ebx
	and $0xf000,%ebx
	push %bx
	mov $sleep_real,%ebx
	and $0xffff,%ebx /*cast off upper bits to avoid overflow*/
	push %bx
	retf


sleep_real:
	mov %cs,%ax #TODO: when segement changes this needs to aswell
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%gs
	mov %ax,%fs

	mov %esp,%eax
	shr $4,%eax
	and $0xf000,%eax
	mov %ax,%ss

	sti
_sleep_loop:
	hlt

	mov $_ticks,%ebx
	movl %es:(%bx),%ecx
	cmp $0,%ecx
	jne _sleep_loop

	cli

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

	jmpl $0x18,$_sleep_32_ret




.global catch_irq08

catch_irq08:
	pusha

	xor %bx,%bx
	mov $_ticks,%ebx
	movl %es:(%bx),%eax
	dec %eax
	movl %eax,%es:(%bx)

	popa

/*Encode a Jump at runtime
 *To the original BIOS's PIT
 *TIMER IRQ cause it may be important
 */
.byte 0xea
.global originalirq8
originalirq8: .long 0x0000


_ticks:
	.long 0x0000

