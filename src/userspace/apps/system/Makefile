CC      := x86_64-elf-gcc
LD      := x86_64-elf-ld
LIBC    := ../../libc

CFLAGS  := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
           -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2   \
           -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11             \
           -I$(LIBC)/include

LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

all: clean system.emx/app.elf

system.emx/app.elf: init.o $(LIBC)/build/crt0.o $(LIBC)/build/libc.a
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o init.o $(LIBC)/build/libc.a -o $@

init.o: init.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBC)/build/crt0.o $(LIBC)/build/libc.a:
	$(MAKE) -C $(LIBC)

clean:
	rm -f *.o system.emx/app.elf

.PHONY: all clean
