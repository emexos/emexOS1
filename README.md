<div align="center">
	<h1>$${\Huge \color{white} \tilde{}~emexOS~\tilde{}}$$</h1>
	<p>
		$${\LARGE \color{red}A \space Simple,\space Modern \space x86\textunderscore 64 \space Operating \space System \space in \space C}$$
	</p>

	<!-- Project badges -->
	<p>
		<img src="https://img.shields.io/github/license/emexos/emexOS1?style=for-the-badge&label=License&labelColor=black&color=white" alt="GitHub License" />
		<img src="https://img.shields.io/github/repo-size/emexos/emexOS1?style=for-the-badge&label=size&labelColor=black&color=white" alt="GitHub repo size" />
		<img src="https://img.shields.io/github/stars/emexos/emexos1?style=for-the-badge&labelColor=black&color=white" alt="GitHub Repo stars" />
	</p>
	<p>
		<a href="https://discord.gg/JFGK4yqSyA">
			<img src="https://img.shields.io/badge/Join%20the%20Discord-Black?style=for-the-badge&logo=discord&logoColor=white&color=black" alt="Discord Badge" />
		</a>
	</p>
</div>

---

## $${\Large About\space emexOS}$$

**emexOS** is a hobbyist, monolithic x86\_64 operating system project with a focus on simplicity, modularity, and modern OS development practices. It is built mostly in C with a touch of assembly, learning towards clarity and educational value. Core goals include:

- Developing a functioning 64-bit modern kernel from scratch
- Clear modular design and codebase, suitable for learning OS concepts
- Clean, approachable interfaces for drivers, FS, graphics, and more
- Easy contribution and extensibility
- Real hardware support (where possible) and robust QEMU emulation

**emexOS** is open for contributors and experimenters — your feedback is always welcome!

---

## $${\Large Key\space Features}$$

- 64-bit x86_64 support
- Modern C codebase (with some assembly for low-level interaction)
- Limine bootloader compatibility
- Modular kernel designed for extensibility
- Early filesystem and ATA disk support (see caution below)
- Text console with password protection and user configuration
- Cross-platform compilation (Linux, Nix, etc.)
- Easy-to-use Makefile and Zig build systems
- Growing documentation and contributor guides

---

## $${\large Build\space Dependencies}$$

To build and run emexOS, you’ll need the following minimal toolchain:
- **x86_64 GCC cross-compiler**  
  <sub>(Primary recommended: emexOS currently only supports 64-bit x86 platforms. Support for new architectures is planned)</sub>
- **[NASM](https://www.nasm.us/)** -- Netwide Assembler, for boot and low-level routines.
- **[QEMU](https://www.qemu.org/)** -- Emulator to test-run OS builds on any machine.
- **[Xorriso](https://www.gnu.org/software/xorriso/)** -- For ISO creation.
- **[Git](https://git-scm.com/)** and **[wget](https://www.gnu.org/software/wget/)** -- To fetch dependencies.

You can also use [Zig](https://ziglang.org/) as an alternative to GCC.

*Optional nice-to-haves:*  
- [GDB](https://www.gnu.org/software/gdb/) for kernel debugging  
- [Bochs](https://bochs.sourceforge.io/) for additional x86 virtualization

---

## $${\large For\space Nix\space Users}$$

If you're on [NixOS](https://nixos.org/) or using Nix package manager:

1. Ensure Nix flakes are enabled (see [nix flakes guide](https://nixos.wiki/wiki/Flakes))
2. Run `nix develop` to enter a shell with all build tools already present (no manual installation needed)
3. For freshest dependencies, update the flake:  
    ```shell
    nix flake update
    ```
4. The devShell includes all required compilers, emulator, linker, assembler and tools.

---

## $${\large Building\space and\space Running}$$

With the dependencies in place, you can compile and test-drive emexOS.

> **:warning: WARNING:**  
> emexOS currently only supports ATA disks.  
> If an ATA disk is detected, emexOS **CAN/WILL DELETE IT WITHOUT ASKING** if `OVERWRITEALL` is set to 1 in the configs (default is 0).  
> Do **NOT** run on bare metal or on drives with important data unless you understand the risks!  
> Use QEMU/virtual machines for experimentation.

### **GNU Make Workflow**
- `make fetchDeps` : Pull in third-party libraries and bootloader packages (uses git/wget)
- `make` : Complete build of emexOS (kernel, ISO, etc.)
- `make run` : Launches emulated OS in QEMU for testing
- `make clean` : Cleans out build artifacts

### **Zig Build Workflow**
- `zig build` : Fetch dependencies, build, run in one command
- `zig build -Dnofetch` : Skip dependency fetch, just build & run
- `zig build -Dclean` : Clean output/cache and then build afresh
- `zig build -Dnofetch -Dclean` : Thorough clean & build, skips fetching

---

## $${\large Project\space Structure}$$

- `kernel/` : The OS kernel (C)
- `boot/`   : Bootloader integration and multiboot setup
- `shared/` : Shared configuration, user/password management, and common headers
- `docs/`   : Documentation, guides, design docs, specs
- `Makefile`, `build.zig` : Build scripts for GNU make and Zig
- Other directories: Drivers, FS, graphics (in-progress), libraries

---

## $${Console}$$

Once emexOS boots in your emulator (or real hardware):
- **Login**: You'll be prompted for a password. Default is `"emex"` (located/configurable in `shared/config/user.h`)
- **User Management**: Change the password by editing the config file and rebuilding.
- **Limitations**: No SMP (multi-core) support yet; emexOS kernel runs on a single CPU only.  
  Graphics, networking, and other advanced features are in progress.

---

## $${\large Contributing}$$

We encourage all contributions!  
If you'd like to add a driver, fix a bug, improve code or docs, please open an [issue](https://github.com/emexos/emexOS1/issues) or [pull request](https://github.com/emexos/emexOS1/pulls).

### Ways to help:
- Report bugs and missing features
- Submit code PRs and documentation improvements
- Discuss features and design in [Discord](https://discord.gg/JFGK4yqSyA)
- Package, test, and extend emexOS

See `CONTRIBUTING.md` for more info (coming soon!)

---

## $${\large Attribution}$$

This OS is a fork-friendly, FOSS project licensed under **GNU GPLv3**.  
**If you fork or modify emexOS, please read [ATTRIBUTION.md](./ATTRIBUTION.md) and retain clear references to "emexOS"** as required.

---

<div align="center">
	<h2>$${Contributors}$$</h2>
	<a href="https://github.com/emexos/emexOS1/graphs/contributors">
	  <img src="https://contrib.rocks/image?repo=emexos/emexOS1" alt="Contributors" />
	</a>
</div>

<br/>

<div align="right">
	<p>$\color{gray}{README \space by\space emexSW,\space Voxi0,\space and\space Bi\ Moz}$</p>
</div>
