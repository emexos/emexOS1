<div align="center">
	<h1>emexOS</h1>
	<h5>a simple 64-bit os written in c</h5>

![Discord Badge](https://img.shields.io/badge/Join%20the%20Discord-Black?style=for-the-badge&logo=discord&logoColor=white&color=black)

![GitHub License](https://img.shields.io/github/license/emexos/emexOS1?style=for-the-badge&label=License&labelColor=black&color=white)
![GitHub repo size](https://img.shields.io/github/repo-size/emexos/emexOS1?style=for-the-badge&label=size&labelColor=black&color=white)
![GitHub Repo stars](https://img.shields.io/github/stars/emexos/emexos1?style=for-the-badge&labelColor=black&color=white)

</div>

## Build Dependencies
For building and compiling emexOS, ensure you have the following installed,
- x86_64 GCC cross-compiler - This is our preferred compiler for the source code. Currently, emexOS only supports 64-bit x86 machines, but we plan to support more architectures in the future.
- [NASM](https://www.nasm.us/) - Assembler.
- [QEMU](https://www.qemu.org/) - Our preferred emulator.
- [Xorriso](https://www.gnu.org/software/xorriso/) - ISO creation.
- [Git](https://git-scm.com/) and [wget](https://www.gnu.org/software/wget/) - To fetch dependencies.

You can use [Zig](https://ziglang.org/) instead of GCC if you prefer to.

## For Nix Users
Ensure you have flakes enabled first and then run `nix develop` to enter the provided development shell which will have all build dependencies and such installed for you. I'd recommend updating the flake using `nix flake update` as it's sometimes not updated for long periods of time.

## Building and Compiling
With all build dependencies installed, you can finally build and run emexOS.
> [!CAUTION]
> If booting emexOS on real hardware, ensure your device has no existing data or else emexOS **WILL DELETE IT WITHOUT ASKING**

**Using gnu-make**
- `make fetchDeps` - Fetch all libraries and such that emexOS depends on e.g. Limine. `git` and `wget` are used for this.
- `make` - Build emexOS.
- `make run` - Emulate emexOS using QEMU.
- `make clean` - Clean up all build outputs.

**Using Zig**
- `zig build` - Fetch, build and run.
- `zig build -Dnofetch` - Build and run.
- `zig build -Dclean` - Remove (cache, output), fetch, build and run.
- `zig build -Dnofetch -Dclean` - Remove (cache, output), build and run.

## Console
After booting, you will be asked for a password which is "emex" by default, to log in. The password can be changed in `shared/config/user.h`.

Currently, no SMP support is available just yet so emexOS runs using only a single core.

<div align="center">
	<h2>Contributors</h2>
	<a href="https://github.com/emexos/emexOS1/graphs/contributors">
	  <img src="https://contrib.rocks/image?repo=emexos/emexOS1" />
	</a>
	<p>Made with [contrib.rocks](https://contrib.rocks).</p>
</div>

---

<div align="center">
	<p><bold>emexOS - by emexSW</bold></p>
</div>