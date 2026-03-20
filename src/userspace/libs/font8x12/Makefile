CC   := x86_64-elf-gcc
AR   := x86_64-elf-ar
LIBC := ../../libc

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2    \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
          -I$(LIBC)/include

all: libfont8x12.a

libfont8x12.a: font8x12.o
	$(AR) rcs $@ $^

font8x12.o: font8x12.c font8x12.h data.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o libfont8x12.a

.PHONY: all clean