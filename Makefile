include common.mk
LIMINE_DIR := $(INCLUDE_DIR)/limine
LIMINE_TOOL := $(LIMINE_DIR)/limine

# Find all C, C++ and Assembly files
SRCS = $(shell find $(SRC_DIR) shared \
	-path "src/userspace" -prune -o \
	\( -name "*.c" -o -name "*.cpp" -o -name "*.asm" \) -print)
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)

.PHONY: all fetchDeps run clean
all: $(ISO)

# Fetch dependencies/libraries
fetchDeps:
	@echo "[DEPS] Fetching dependencies/libraries"
	@mkdir -p $(INCLUDE_DIR)
	#@mkdir -p $(BUILD_DIR)/src/kernel/console/functions
	@echo "[DEPS] Fetching Limine"
	@rm -rf $(INCLUDE_DIR)/limine
	@git clone https://codeberg.org/Limine/Limine.git --branch=v10.3.0-binary --depth=1 $(INCLUDE_DIR)/limine
	@echo "[DEPS] Building limine binary"
	@$(MAKE) -C $(INCLUDE_DIR)/limine

disk:
	@mkdir -p $(DISK_DIR)
	@touch $(DISK_DIR)/disk.img
# Kernel binary
$(BUILD_DIR)/kernel.elf: src/kernel/linker.ld $(OBJS)
	@mkdir -p $(dir $@)
	$(VLD) $(LDFLAGS) -T $< $(OBJS) -o $@

# Build userspace first
userspace:
	@$(MAKE) -C src/userspace

# Ensure host limine tool exists (Linux/macOS binary).
$(LIMINE_TOOL):
	@$(MAKE) -C $(LIMINE_DIR)

# Create bootable ISO
$(ISO): limine.conf $(BUILD_DIR)/kernel.elf disk userspace $(LIMINE_TOOL)
	@echo "[ISO] Creating bootable image..."
	@rm -rf $(ISODIR)
	@rm -f $(DISK_DIR)/initrd.cpio
	@mkdir -p $(ISODIR)/boot/limine $(ISODIR)/EFI/BOOT
	@mkdir -p $(ISODIR)/boot
	@mkdir -p $(ISODIR)/boot/ui
	@mkdir -p $(ISODIR)/boot/ui/fonts
	@mkdir -p $(ISODIR)/boot/ui/assets
	@mkdir -p $(ISODIR)/boot/modules
	@mkdir -p $(ISODIR)/boot/keymaps
	@mkdir -p $(ISODIR)/boot/images
	@mkdir -p $(ISODIR)/boot/programs
	@cp $(BUILD_DIR)/kernel.elf $(ISODIR)/boot/kernel_a.elf
	@cp $(BUILD_DIR)/kernel.elf $(ISODIR)/boot/kernel_b.elf
	@cp $< $(ISODIR)/boot/limine/
	@cp $(addprefix $(INCLUDE_DIR)/limine/limine-, bios.sys bios-cd.bin uefi-cd.bin) $(ISODIR)/boot/limine/
	@cp $(addprefix $(INCLUDE_DIR)/limine/BOOT, IA32.EFI X64.EFI) $(ISODIR)/EFI/BOOT/

	@echo "[MOD] creating executables..."
	@mkdir -p $(DISK_DIR)/rd/user/apps
	@cp -r src/userspace/apps/hello/hello.emx $(DISK_DIR)/rd/user/apps/

	@echo "[MOD] creating initrd.cpio..."
	@chmod +x tools/initrd.sh
	./tools/initrd.sh
	@cp $(DISK_DIR)/initrd.cpio $(ISODIR)/boot/


	@xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$(ISODIR) -o $@ 2>/dev/null
	@$(LIMINE_TOOL) bios-install $@
	@echo "------------------------"
	@echo "[OK]  $@ created"

# Run/Emulate OS
run: $(ISO)
	@echo "[QEMU]running $(OS_NAME).iso "
	@echo
	@qemu-system-x86_64 \
		-M pc \
		-cpu qemu64 \
		-m 512 \
		-drive if=pflash,format=raw,readonly=on,file=uefi/OVMF_CODE.fd \
		-drive if=pflash,format=raw,file=uefi/OVMF_VARS.fd \
		-drive file=$(DISK_IMG),format=raw,if=ide,index=0 \
		-cdrom $< \
		-serial stdio 2>&1 \
		#-no-reboot \
		#-no-shutdown

run_usb: $(ISO)
	@chmod +x run_xhci.sh
	./run_xhci.sh
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
