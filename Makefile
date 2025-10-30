#.PHONY: all clean run iso

CC = x86_64-elf-gcc
CXX = x86_64-elf-g++
LD = x86_64-elf-ld
AS = nasm

CFLAGS = -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector \
         -fno-stack-check -fno-lto -fno-PIE -fno-pic -m64 -march=x86-64 \
         -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone \
         -mcmodel=kernel -Ithird_party/limine -Isrc -nostdlib

CXXFLAGS = -Wall -Wextra -std=c++17 -ffreestanding -fno-stack-protector \
           -fno-stack-check -fno-lto -fno-PIE -fno-pic -m64 -march=x86-64 \
           -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone \
           -mcmodel=kernel -Ithird_party/limine -Isrc -nostdlib \
           -fno-exceptions -fno-rtti

LDFLAGS = -nostdlib -static -no-pie -z text -z max-page-size=0x1000 \
          -T src/kernel/linker.ld

ASFLAGS = -f elf64

# C Source Files
KERNEL_C_SRC = src/kernel/kernel.c \
               src/kernel/console/console.c \
               src/kernel/console/shell_screen.c \
               src/kernel/console/functions/text.c \
               src/kernel/console/functions/system.c \
               src/kernel/console/functions/cmos.c \
               src/kernel/include/logo.c \
               src/kernel/cpu/idt.c \
               src/kernel/cpu/isr.c \
               src/kernel/cpu/irq.c \
               src/kernel/cpu/gdt.c \
               src/kernel/cpu/timer.c \
               src/kernel/mem_manager/physmem.c \
               src/kernel/proc/process.c \
               src/kernel/proc/scheduler.c \
               src/libs/graphics/graphics.c \
               src/libs/graphics/standard/screen.c \
               src/libs/graphics/draw.c \
               src/libs/string/string.c \
               src/libs/print/print.c \
               src/libs/memory/mem.c \
               src/drivers/ps2/keyboard/keyboard.c \
               src/drivers/cmos/cmos.c \

# Assembly Files
KERNEL_ASM = src/kernel/cpu/idt.asm \
             src/kernel/cpu/isr.asm \
             src/kernel/cpu/irq.asm \
             src/kernel/cpu/gdt.asm
             #\
             #src/kernel/proc/scheduler.asm

KERNEL_C_OBJ = $(patsubst src/%.c,build/%.o,$(KERNEL_C_SRC))
KERNEL_CXX_OBJ = $(patsubst src/%.cpp,build/%.o,$(KERNEL_CXX_SRC))
KERNEL_ASM_OBJ = $(patsubst src/%.asm,build/%_asm.o,$(KERNEL_ASM))
KERNEL_OBJ = $(KERNEL_C_OBJ) $(KERNEL_CXX_OBJ) $(KERNEL_ASM_OBJ)

KERNEL = kernel.elf
ISO = doccrOS.iso

all: $(ISO)

# C Compilation
build/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "  [CC]  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# C++ Compilation
build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@echo "  [CXX] $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Assembly
build/%_asm.o: src/%.asm
	@mkdir -p $(dir $@)
	@echo "  [AS]  $<"
	@$(AS) $(ASFLAGS) $< -o $@

# Linking
$(KERNEL): $(KERNEL_OBJ)
	@echo "  [LD]  $(KERNEL)"
	@$(LD) $(LDFLAGS) $(KERNEL_OBJ) -o $(KERNEL)

# ISO Creation
$(ISO): $(KERNEL)
	@echo "  [ISO] Creating bootable image..."
	@rm -rf iso_root
	@mkdir -p iso_root/boot/limine iso_root/EFI/BOOT
	@cp $(KERNEL) iso_root/boot/
	@cp limine.conf iso_root/boot/limine/
	@cp third_party/limine/limine-bios.sys iso_root/boot/limine/
	@cp third_party/limine/limine-bios-cd.bin iso_root/boot/limine/
	@cp third_party/limine/limine-uefi-cd.bin iso_root/boot/limine/
	@cp third_party/limine/BOOTX64.EFI iso_root/EFI/BOOT/
	@cp third_party/limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	@xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
	        -no-emul-boot -boot-load-size 4 -boot-info-table \
	        --efi-boot boot/limine/limine-uefi-cd.bin \
	        -efi-boot-part --efi-boot-image --protective-msdos-label \
	        iso_root -o $(ISO) 2>/dev/null
	@echo "  [OK]  $(ISO)"

run: $(ISO)
	@echo "  [RUN] Starting QEMU..."
	@qemu-system-x86_64 \
		-cdrom $(ISO) \
		-m 256M \
		#-enable-kvm \
		#-device qemu-xhci,id=xhci \
		#-device usb-kbd \
		#-device usb-mouse \
	2>/dev/null || \
	qemu-system-x86_64 \
		-cdrom $(ISO) \
		-m 256M \
		#-device qemu-xhci,id=xhci \
		#-device usb-kbd \
		#-device usb-mouse

clean:
	@echo "  [CLR] Removing build files..."
	@rm -rf build $(KERNEL) $(ISO) iso_root
	@echo "  [OK]  Clean complete"
