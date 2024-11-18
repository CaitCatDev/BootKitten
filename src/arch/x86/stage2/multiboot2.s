.code64

.global _boot_multiboot2
_boot_multiboot2:
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

	pushq $0x18
	pushq $_boot_multiboot2_32
	lretq

.code32
_boot_multiboot2_32:
	mov $0x20,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs
	
	mov %cr0,%eax
	and $0xff,%eax
	mov %eax,%cr0

	mov $_boot_info,%ebx
	mov $0x36d76289,%eax

	/*Jump to kernel*/
	/*TODO: Get this at runtime not hard coded*/
	cli
	jmp $0x18,$0x100020

/*We shouldn't really hard code this but it'll work for a simple test*/
_boot_info:
	.long 39
	.long 0x00
	/*Bootloader Name*/
	.long 2
	.long 24 
	.asciz "KittyBoot V0.1 "
	/*End Tag*/
	.long 0
	.long 8
