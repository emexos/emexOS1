include common.mk

.PHONY: all clean run iso

KERNEL = kernel.elf
ISO = emexOS.iso

# Source files
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
               src/drivers/cmos/cmos.c

KERNEL_ASM = src/kernel/cpu/idt.asm \
             src/kernel/cpu/isr.asm \
             src/kernel/cpu/irq.asm \
             src/kernel/cpu/gdt.asm

KERNEL_C_OBJ  = $(patsubst src/%.c,build/%.o,$(KERNEL_C_SRC))
KERNEL_ASM_OBJ = $(patsubst src/%.asm,build/%_asm.o,$(KERNEL_ASM))
KERNEL_OBJ = $(KERNEL_C_OBJ) $(KERNEL_ASM_OBJ)

all: $(ISO)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(VCXX) $(CXXFLAGS) -c $< -o $@

build/%_asm.o: src/%.asm
	@mkdir -p $(dir $@)
	$(VAS) $(ASFLAGS) $< -o $@


$(KERNEL): $(KERNEL_OBJ)
	$(VLD) $(LDFLAGS) $(KERNEL_OBJ) -o $(KERNEL)


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
#   @echo "  [RUN]  emexOS"
	@chmod +x ./run_qemu.sh
	@./run_qemu.sh $(ISO)

clean:
	@echo "  [CLR] Cleanup..."
	@rm -rf build $(KERNEL) $(ISO) iso_root
	@echo "  [OK]"
