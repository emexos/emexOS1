CC   := x86_64-elf-gcc
AR   := x86_64-elf-ar
LIBC := ../../libc

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2    \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
          -I$(LIBC)/include

all: libvad.a

libvad.a: vad.o
	$(AR) rcs $@ $^

vad.o: vad.c vad.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o libvad.a

.PHONY: all clean