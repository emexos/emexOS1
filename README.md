<div align="center">
	<h1> emexOS </h1>


 a simple x86_64 Operating System in C made from scratch

![GitHub License](https://img.shields.io/github/license/emexos/emexOS1?style=for-the-badge&label=License&labelColor=black&color=white)
![GitHub repo size](https://img.shields.io/github/repo-size/emexos/emexOS1?style=for-the-badge&label=size&labelColor=black&color=white)
![GitHub Repo stars](https://img.shields.io/github/stars/emexos/emexos1?style=for-the-badge&labelColor=black&color=white)


<a href="https://discord.gg/JFGK4yqSyA">
	<img src="https://img.shields.io/badge/Join%20the%20Discord-Black?style=for-the-badge&logo=discord&logoColor=white&color=black" alt="Discord Badge" />
</a>

(emexOS is not linux based)

</div>

## Build Dependencies 
For building and compiling emexOS, ensure you have the following installed:
- x86_64 GCC cross-compiler - This is our preferred compiler for the source code. Currently, emexOS only supports 64-bit x86 machines, but we plan to support more architectures in the future.
- [NASM](https://www.nasm.us/) - Assembler.
- [QEMU](https://www.qemu.org/) - Our preferred emulator.
- [Xorriso](https://www.gnu.org/software/xorriso/) - ISO creation.
- [Git](https://git-scm.com/) and [wget](https://www.gnu.org/software/wget/) - To fetch dependencies.
- cpio (use brew install cpio) - For the RAM disk.

You can use [Zig](https://ziglang.org/) instead of GCC if you prefer to.

## For Nix Users 
Ensure you have flakes enabled first, then run `nix develop` to enter the provided development shell which will have all build dependencies and such installed for you. I'd recommend updating the flake using `nix flake update` as it's sometimes not updated for long periods of time.

## Building and Compiling 
With all build dependencies installed, you can finally build and run emexOS.
> [!CAUTION]
> emexOS currently only supports ATA disks, if such a disk is detected emexOS **WILL/CAN DELETE IT WITHOUT ASKING** (if you have OVERWRITEALL set to 1 in the configs which is by default 0).

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
After booting, you will be asked for a password and username, both are `emex` by default. The password can be changed in `shared/config/user.h`.

Currently, no SMP support is available just yet so emexOS runs using only a single core.

<div align="center">
	<h2> Contributors </h2>
	<a href="https://github.com/emexos/emexOS/graphs/contributors">
	  <img src="https://contrib.rocks/image?repo=emexos/emexOS" />
	</a>
</div>

</br>

<div align="right">
	<p>readme by emexSW, Voxi0 and Bi Moz </p>
</div>
