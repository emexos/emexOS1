CC  := x86_64-elf-gcc
AR  := x86_64-elf-ar
LIBC := ../../libc

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2    \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
          -I$(LIBC)/include

all: libdesktop.a

libdesktop.a: libdesktop.o
	$(AR) rcs $@ $^

libdesktop.o: libdesktop.c libdesktop.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o libdesktop.a

.PHONY: all clean