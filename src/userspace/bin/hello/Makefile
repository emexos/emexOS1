CC      := x86_64-elf-gcc
LD      := x86_64-elf-ld
LIBC    := ../../libc

CFLAGS  := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
           -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2   \
           -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11             \
           -I$(LIBC)/include

LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

OUT := hello.elf

all: clean $(OUT)

$(OUT): hello.o $(LIBC)/build/crt0.o $(LIBC)/build/libc.a
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o hello.o $(LIBC)/build/libc.a -o $@

hello.o: hello.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBC)/build/crt0.o $(LIBC)/build/libc.a:
	$(MAKE) -C $(LIBC)

clean:
	rm -f *.o $(OUT)

.PHONY: all clean
