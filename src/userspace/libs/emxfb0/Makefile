CC   := x86_64-elf-gcc
AR   := x86_64-elf-ar
LIBC := ../../libc

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2    \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
          -I$(LIBC)/include

all: libemxfb0.a

libemxfb0.a: emxfb0.o
	$(AR) rcs $@ $^

emxfb0.o: emxfb0.c emxfb0.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o libemxfb0.a

.PHONY: all clean