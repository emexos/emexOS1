# Emulating emexOS Binary with QEMU

This guide explains in detail how to emulate and run **emexOS** using QEMU, a virtual machine and hardware emulator. This approach is especially useful for testing, development, or evaluation on any host OS that runs QEMU.

---

## Step 1: Install QEMU

Install QEMU if you don't already have it. See distribution-specific setup:
- [QEMU on Arch Linux](./QEMU_Arch.md)
- [QEMU on Debian/Ubuntu](./QEMU_Debian.md)

Ensure you install both the `qemu-system-x86_64` and any GUI or management tools you prefer.

---

## Step 2: Download emexOS ISO

1. Visit the [emexOS Releases page](https://github.com/emexos/emexOS1/releases).
2. Download the latest release `.iso` image (e.g., `emexOS1-0.1.iso`).
3. Save the ISO to a known location on your system.

---

## Step 3: Create a Virtual Machine

You have two main options for launching emexOS in QEMU:

### **A. Command Line (Recommended for Developers)**

Open a terminal and run:

```sh
qemu-system-x86_64 \
    -cdrom /path/to/emexOS1-0.1.iso \
    -m 512 \
    -smp 2 \
    -boot d \
    -serial stdio \
    -display sdl
```

**Options explained:**
- `-cdrom`: Path to the downloaded emexOS ISO
- `-m`: Amount of RAM (in MB, minimum 512 recommended)
- `-smp`: Number of CPU cores (e.g. 2)
- `-serial stdio`: Connect guest serial port to your terminal for debugging
- `-display sdl`: Use a windowed graphical display (alternatively use `-display gtk` or `-curses` for text-only)

**Extra options you may consider:**
- `-enable-kvm` for hardware acceleration
- `-drive file=disk.img,format=raw` to attach a virtual hard disk

---

### **B. Graphical Virtual Machine Manager (GUI)**

If you prefer using a Visual Virtual Machine Manager (like `virt-manager`):

1. **Open** your Virtual Machine Manager application.
2. Click **"Create a new virtual machine"**.
3. Select **"Local install media (ISO)"**.
4. For installation media, click **"Browse"** and select your downloaded emexOS ISO.
5. Uncheck **"Automatically detect from the installation media"**.
6. Manually specify the OS type as **"Generic Linux 2024"** or similar when prompted.
7. Assign **at least 512MB RAM** and **2 CPU cores** (more if available).
8. You can choose **whether or not to add a virtual storage disk** (emexOS will boot and run in memory regardless).
9. Leave other settings at their defaults, unless you need to customize hardware.
10. Click **"Finish"** to complete the setup.

---

## Additional Tips

- For troubleshooting, use `-serial stdio` to view kernel logs and outputs directly in the terminal.
- Adjust RAM and CPU core count as needed for your system and performance.
- Storage is optional; emexOS can also boot in a live environment.
- See the main [Building emexOS from Source](./Building_emexOS_from_source.md) guide if you want to build the ISO yourself.

---

**For additional configuration details, consult the official [QEMU documentation](https://wiki.qemu.org/Documentation/Networking) or visit the [emexOS wiki](https://github.com/emexos/emexOS1/wiki).**
