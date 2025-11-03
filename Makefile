include common.mk

# Find all C, C++ and Assembly files
SRCS = $(shell find $(SRC_DIR) -name "*.c" -or -name "*.cpp" -or -name "*.asm")
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)

.PHONY: all clean run iso
all: $(ISO)

# Kernel binary
$(BUILD_DIR)/kernel.elf: src/kernel/linker.ld $(OBJS)
	$(VLD) $(LDFLAGS) -T $< $(OBJS) -o $@

# Bootable OS ISO
$(ISO): $(BUILD_DIR)/kernel.elf
	@echo "  [ISO] Creating bootable image..."
	@rm -rf iso_root
	@mkdir -p iso_root/boot/limine iso_root/EFI/BOOT
	@cp $(BUILD_DIR)/kernel.elf iso_root/boot/
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
	@echo "  ------------------------"
	@echo "  [OK]  $(ISO) created"

# Run/Emulate OS
run: $(ISO)
	@qemu-system-$(ARCH) -m 512 -cdrom $<

# Compilation rules
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(VCXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	$(VAS) $(ASFLAGS) $< -o $@

# Clean all build output
clean:
	@echo "[CLR] Cleaning..."
	@rm -rf $(BUILD_DIR)
	@echo "[OK]"
