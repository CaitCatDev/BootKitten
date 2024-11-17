.POSIX:
.SUFFIXES: .c .s .o
AS=as
CC=clang
LD=ld
OBJCOPY=objcopy

CFLAGS=-I ./includes/ -I ./includes/arch/x86/ -ffreestanding -fno-builtin -mno-red-zone -fno-PIC -fno-PIE

MBR_OBJS=./src/arch/x86/stage1/mbr.o
MBR_ELF=./mbr.elf
MBR_BIN=./mbr.bin

ST2_OBJS=./src/arch/x86/stage2/start.o ./src/arch/x86/stage2/int86.o ./src/arch/x86/stage2/main.o ./src/arch/x86/stage2/intterupts.o ./src/arch/x86/stage2/multiboot2.o
ST2_BIN=./stage2.bin
ST2_ELF=./stage2.elf

BIOS_IMG=./bios.img

all: $(BIOS_IMG)

$(BIOS_IMG): $(MBR_BIN) $(ST2_BIN)
	dd if=/dev/zero of=$@ count=102400
	dd if=$(MBR_BIN) of=$@ count=1 conv=notrunc
	dd if=$(ST2_BIN) of=$@ conv=notrunc seek=1 bs=512


test: $(BIOS_IMG)
	qemu-system-x86_64 -d int $(BIOS_IMG)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	$(AS) -o $@ $<

$(MBR_ELF): $(MBR_OBJS)
	$(LD) -T ./ld-scripts/mbr.ld -o $@ $(MBR_OBJS)

$(MBR_BIN): $(MBR_ELF)
	$(OBJCOPY) -O binary -j .text $(MBR_ELF) $@

$(ST2_ELF): $(ST2_OBJS)
	$(LD) -T ./ld-scripts/stage2.ld -o $@ $(ST2_OBJS)

$(ST2_BIN): $(ST2_ELF)
	$(OBJCOPY) -O binary -j .text -j .data -j .rodata $(ST2_ELF) $@

clean:
	rm *.elf *.bin $(MBR_OBJS) $(ST2_OBJS) *.img
