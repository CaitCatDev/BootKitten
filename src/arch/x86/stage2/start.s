.code16

.text
.extern cmain
.global _start

_start:
	cli

	lgdt _GDTR

	mov %cr0,%eax
	or $1,%eax
	mov %eax,%cr0
	
	jmpl $0x18,$_pm_start

.code32
_pm_start:
	mov $0x20,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs


_is_x86_64: #Check if this CPU is 64 or just 32bit
	pushfl
	pop %eax
	mov %eax,%ebx
	xor $1<<21,%eax
	push %eax
	popfl
	pushfl
	pop %eax
	cmp %eax,%ebx
	jz _no_cpuid

	mov $0x80000000,%eax
	cpuid
	cmp $0x80000001,%eax
	jb _no_cpuid

	mov $0x80000001,%eax
	cpuid
	test $1<<29,%edx
	jz _no_lm

_paging_init32:
	xor %eax,%eax
	mov $0x1200,%ecx
	mov $pml4,%edi
	rep stosl

	mov $pml4,%edi
	mov %edi,%cr3
	mov $pdp,%ebx
	mov %ebx,(%edi)
	orl $0x3,(%edi)

	add $0x1000,%edi
	add $0x1000,%ebx
	
	mov %ebx,(%edi)
	orl $0x3,(%edi)

	add $0x1000,%edi
	add $0x1000,%ebx
	
	mov %ebx,(%edi)
	orl $0x3,(%edi)
	add $0x1000,%ebx
	add $8,%edi
	mov %ebx,(%edi)
	orl $0x3,(%edi)	
	
	mov $pt1,%edi
	xor %ebx,%ebx
	orl $0x3,%ebx
	mov $1024,%ecx

	.fill_tables:
	mov %ebx,(%edi)
	add $8,%edi
	add $0x1000,%ebx
	loop .fill_tables

	mov $0x20,%eax
	mov %eax,%cr4

	mov $0xC0000080,%ecx
	rdmsr
	or $0x100,%eax
	wrmsr

	mov %cr0,%eax
	or $1<<31,%eax
	mov %eax,%cr0
	jmp $0x28,$_lm_start
_no_lm:
	mov $_lm_error_string,%esi
	mov $0x4f,%ah
	mov $0xb8000,%edi
	jmp _puts32

_no_cpuid:
	mov $_cpuid_error_string,%esi
	mov $0x4f,%ah
	mov $0xb8000,%edi

_puts32:
	lodsb
	cmp $0x00,%al
	je _hlt_forever
	stosw
	jmp _puts32

_hlt_forever:
	cli
	hlt
	jmp _hlt_forever

.code64
_lm_start:
	mov $0x30,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%gs
	mov %ax,%fs

	mov $0x7c00,%rsp

	jmp cmain
	
.global halt
.global no_interrupts
halt:
	hlt
	ret

no_interrupts:
	cli
	ret

.section .rodata
_lm_error_string:
	.asciz "Long Mode Unsupported"

_cpuid_error_string:
	.asciz "CPUID Unsupported"

.data
_GDT:
	.null:
	.quad 0x00
	.code16:
	.word 0xffff
	.word 0x0000
	.byte 0x00
	.byte 0x9a
	.byte 0x8f
	.byte 0x00
	.data16:
	.word 0xffff
	.word 0x0000
	.byte 0x00
	.byte 0x92
	.byte 0x8f
	.byte 0x00
	.code32:
	.word 0xffff
	.word 0x0000
	.byte 0x00
	.byte 0x9a
	.byte 0xcf
	.byte 0x00
	.data32:
	.word 0xffff
	.word 0x0000
	.byte 0x00
	.byte 0x92
	.byte 0xcf
	.byte 0x00
	.code64:
	.word 0x0000
	.word 0x0000
	.byte 0x00
	.byte 0x9a
	.byte 0x20
	.byte 0x00
	.data64:
	.word 0x0000
	.word 0x0000
	.byte 0x00
	.byte 0x92
	.byte 0x00
	.byte 0x00	
.end:

_GDTR:
	.word 0x37
	.long _GDT

.bss
.align 0x1000
.lcomm pml4,0x1000
.lcomm pdp,0x1000
.lcomm pd,0x1000
.lcomm pt1,0x1000
.lcomm pt2,0x1000
