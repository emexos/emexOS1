# Building emexOS from Source

This guide covers building **emexOS** from source for contributors and enthusiasts. The instructions are tailored for maximum reproducibility, using clear code blocks and step-by-step explanations optimized for GitHub readability.

---

## Requirements

You may choose to build with standard Linux development tools (Debian-based recommended), or with the Zig toolchain for a distribution-agnostic workflow. Both approaches are described below.

---

## Option 1: Building on Debian-based Distributions

**Recommended for most users familiar with standard Linux development tools.**

### Step 1: Install Dependencies

Open a terminal and install the required packages:

```sh
sudo apt update
sudo apt install \
    git nasm qemu-system-x86 xorriso \
    build-essential binutils gcc make pkg-config
```

> **Note:**  
> If you encounter missing package errors, ensure you are using a current, supported Debian or Ubuntu release.

### Step 2: Download the Source Code

Download the latest release archive from [GitHub Releases](https://github.com/emexos/emexOS1/releases).

For example, to obtain version `emexOS1-0.1.zip`:

```sh
# Replace the URL below with the link to the desired release
wget https://github.com/emexos/emexOS1/releases/download/v0.1/emexOS1-0.1.zip
unzip emexOS1-0.1.zip
cd emexOS1-0.1
```

### Step 3: Build emexOS

Run the following commands to set up dependencies and build the project:

```sh
make fetchDeps
make all
```

These commands:
- Automatically fetch and configure all required third-party dependencies
- Build the OS kernel, user tools, and image files
- Generate the final bootable ISO in the `build/` directory

### Step 4: Run emexOS in QEMU

After a successful build, launch emexOS using QEMU:

```sh
qemu-system-x86_64 -cdrom build/emexOS.iso -m 512 -smp 2 -serial stdio -boot d
```

**Tips:**
- You may adjust `-m` (memory) and `-smp` (CPU cores) as needed.
- For graphical output: use the `-display` flag described in [QEMU documentation](https://wiki.qemu.org/Documentation/Networking).

---

## Option 2: Building with Zig (Cross-platform, Easy Setup)

**Best if you want a simple, self-contained build that works across distributions.**

### Step 1: Install Zig

Follow the official [Zig installation guide](https://zig.guide/getting-started/installation/) appropriate for your OS.

### Step 2: Acquire the Source and Build

Download and extract the latest release as above. Then:

```sh
cd emexOS1-0.1
zig build
```

This will automatically handle all build steps and produce the final ISO image in the output directory.

---

## Additional Notes

- For further customization or troubleshooting, see [`docs/`](.) and the [project wiki](https://github.com/emexos/emexOS1/wiki).
- If you have issues, please [open an issue](https://github.com/emexos/emexOS1/issues) with your build log and system details.
- Advanced users may refer to `Makefile` and `build.zig` for extending build options.

---

**Happy hacking!**
