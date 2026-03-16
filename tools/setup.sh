#!/bin/bash
DEPS_COMMON="nasm xorriso qemu git wget make"

if [[ "$OSTYPE" == "darwin"* ]]; then
    brew install $DEPS_COMMON x86_64-elf-gcc x86_64-elf-binutils

elif command -v pacman &>/dev/null; then
    sudo pacman -Syu --noconfirm $DEPS_COMMON qemu-system-x86 base-devel edk2-ovmf
    yay -S --noconfirm x86_64-elf-gcc x86_64-elf-binutils

elif command -v apt-get &>/dev/null; then
    sudo apt-get update
    sudo apt-get install -y $DEPS_COMMON qemu-system-x86 ovmf build-essential
    echo "x86_64-elf-gcc is not in apt. Install manually: https://wiki.osdev.org/GCC_Cross-Compiler"

elif command -v dnf &>/dev/null; then
    sudo dnf install -y $DEPS_COMMON qemu-system-x86 edk2-ovmf gcc
    echo "x86_64-elf-gcc is not in dnf. Install manually: https://wiki.osdev.org/GCC_Cross-Compiler"

else
    echo "Unsupported system. Install manually: nasm xorriso qemu x86_64-elf-gcc x86_64-elf-binutils git wget make"
    exit 1
fi

echo "finish :)"