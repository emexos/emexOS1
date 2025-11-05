OS_NAME ?= emexOS
ARCH ?= x86_64

# Build toolchain
CC := $(ARCH)-elf-gcc
CXX := $(ARCH)-elf-g++
LD := $(ARCH)-elf-ld
AS := nasm
OBJCOPY := $(ARCH)-elf-objcopy
VCC = @echo "[CC] $<" && $(CC)
VCXX = @echo "[CXX] $<" && $(CXX)
VAS = @echo "[AS] $<" && $(AS)
VLD = @echo "[LD] $@" && $(LD)

# Compiler Flags
COMMON_FLAGS ?= -ffreestanding -nostdlib -fno-stack-protector -fno-lto \
                -fno-PIE -fno-pic -m64 -march=x86-64 -mno-80387 -mno-mmx \
                -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Wall -Wextra
CFLAGS ?= $(COMMON_FLAGS) -std=gnu11
CXXFLAGS ?= $(COMMON_FLAGS) -std=c++17 -fno-exceptions -fno-rtti
LDFLAGS ?= -nostdlib -static -no-pie -z text -z max-page-size=0x1000 
ASFLAGS ?= -f elf64

# Directories and files
SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIR := include
ISODIR := $(BUILD_DIR)/isodir
ISO := $(BUILD_DIR)/$(OS_NAME).iso
