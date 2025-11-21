# Building emexOS from source

For this endeavor, we suggest to use Debian-based distributions.

## 1. Using Debian-based distribtuions to build

At first install the following packages.

```
sudo apt update
sudo apt install git nasm qemu-system-x86 xorriso build-essential binutils gcc make pkg-config
```
Download the latest source code from the [emexOS releases](https://github.com/emexos/emexOS1/releases).
Next you have to unpack and ```cd``` into the unarchived folder.

```
unzip /path/to/emexOS1-0.1.zip
cd emexOS1-0.1
```

Finally, just run the following commands to build emexOS.

```
make fetchDeps
make all
```

To run your freshly built emexOS, use the following commands.

```
qemu-system-x86_64 -cdrom build/emexOS.iso -m 512 -smp 2 -serial stdio -boot d
```

### 2. Using Zig to build

Independent of any package manager or linux distro, using zig to build emexOS is the easiest option.

Firstly, get zig working in your workstation by following [this guide](https://zig.guide/getting-started/installation/).

Then, simply download the latest source code from the [emexOS releases](https://github.com/emexos/emexOS1/releases) and ```cd``` into the unzipped file and do ```zig build```.

```
cd emexOS1-0.1
zig build
```
