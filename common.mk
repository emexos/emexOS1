ARCH       ?= x86_64-elf

ifeq "$(origin CC)" "default"
CC         := $(ARCH)-gcc
endif
ifeq "$(origin CXX)" "default"
CXX        := $(ARCH)-g++
endif

LD         := $(ARCH)-ld
AS         := nasm
AR         := $(ARCH)-ar
OBJCOPY    := $(ARCH)-objcopy

BUILD_DIR  := build

# Compiler Flags
COMMON_FLAGS ?= -ffreestanding -nostdlib -fno-stack-protector -fno-lto \
                -fno-PIE -fno-pic -m64 -march=x86-64 -mno-80387 -mno-mmx \
                -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Wall -Wextra

CFLAGS      ?= $(COMMON_FLAGS) -std=gnu11
CXXFLAGS    ?= $(COMMON_FLAGS) -std=c++17 -fno-exceptions -fno-rtti
LDFLAGS     ?= -nostdlib -static -no-pie -z text -z max-page-size=0x1000 \
               -T src/kernel/linker.ld
ASFLAGS     ?= -f elf64


ifeq ($(V),1)
  VCC   = $(CC)
  VCXX  = $(CXX)
  VAS   = $(AS)
  VLD   = $(LD)
else
  VCC   = @echo "  [CC]  $<" && $(CC)
  VCXX  = @echo "  [CXX] $<" && $(CXX)
  VAS   = @echo "  [AS]  $<" && $(AS)
  VLD   = @echo "  [LD]  $@" && $(LD)
endif
