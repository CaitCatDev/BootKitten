.code16

##
# DAP LAYOUT 
# (Offset, Size, Name)
# 0x00, 1byte, Packet Size 0x10 or 0x18 if EDD-3.0 is used
# 0x01, 1byte, Reserved must be 0
# 0x02, 2bytes, Blocks to read
# 0x04, 2bytes, Destination offset
# 0x06, 2bytes, Destination segment
# 0x08, 8bytes, 64bit LBA
# 0x10, 8bytes, 64bit Destination address(we don't use this) only with EDD 3.0
##
.set DAP, 0x1000

.text
.global _start

_start:
	jmp _real_start
	nop

_bpb:
	.org 90

##
# Code here inits the CPU into a known state
# as at the moment the BIOS can essentially put
# the CPU into any state that it wants and we don't
# know what state it's put registers in.
##
_real_start:
	cli #Disable ints
	xor %ax,%ax #Clear AX
	#Clear all the segment regs we use
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss

	mov $0x7c00,%sp #Set up a stack
	cld #Clear the direction flag
	jmp $0x0000,$_clear_cs

_clear_cs:
	sti

	or $3,%al
	int $0x10 #Set a known video mode
	
	pushw $0x1000
	pop %es
	xor %di,%di
	mov $1,%eax
	xor %ebp,%ebp
	call _read_disk
	jmp $0x1000,$0x0000
_endless_loop:
	cli
	hlt
	jmp _endless_loop

disk_read_error:
	incb _err_code

print_error:
	pushw $0xb800
	popw %es
	
	movw $'E'+0x1f00,%ax
	movw %ax,%es:0x0000
	movb _err_code,%al
	movw %ax,%es:0x0002

	jmp _endless_loop

##
# EAX = LBA low 32bit
# EBX = LBA high 32bit
# DL = Disk to read
# CX = Bytes to read
# DI = Destination Offset
# ES = Destination Segment
##
_read_disk:
	#Construct the DAP
	movw $0x0010,DAP #Reserved and size bytes
	movw $0x0010,DAP+0x2 #Blocks to read
	movw %di,DAP+0x4 #Offset Dest
	movw %es,DAP+0x6 #Segment Dest
	movl %eax,DAP+0x8 #LBA low
	movl %ebp,DAP+0xc #LBA high

	movb $0x42,%ah
	movw $DAP,%si
	int $0x13

	jc disk_read_error

	ret
.org 429
_err_code: .byte '0'
_stage2_sz: .word 0x00
_stage2_lba: .quad 0x01
.org 440
	#mbr part tables
.org 510
.word 0xaa55
