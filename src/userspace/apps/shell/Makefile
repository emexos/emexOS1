CC      := x86_64-elf-gcc
LD      := x86_64-elf-ld
LIBC    := ../../libc

CFLAGS  := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
           -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2   \
           -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11             \
           -I$(LIBC)/include

LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

all: clean shell.emx/app.elf

shell.emx/app.elf: shell.o $(LIBC)/build/crt0.o $(LIBC)/build/libc.a
	mkdir -p shell.emx
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o shell.o $(LIBC)/build/libc.a -o $@

shell.o: shell.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBC)/build/crt0.o $(LIBC)/build/libc.a:
	$(MAKE) -C $(LIBC)

clean:
	rm -f *.o shell.emx/app.elf

.PHONY: all clean
