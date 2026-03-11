CC      := x86_64-elf-gcc
LD      := x86_64-elf-ld
LIBC    := ../../libc

CFLAGS  := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
           -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2    \
           -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
           -I$(LIBC)/include

LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

all: clean gui.emx/app.elf

gui.emx/app.elf: gui.o $(LIBC)/build/crt0.o $(LIBC)/build/libc.a
	mkdir -p gui.emx
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o gui.o $(LIBC)/build/libc.a -o $@

gui.o: gui.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBC)/build/crt0.o $(LIBC)/build/libc.a:
	$(MAKE) -C $(LIBC)

clean:
	rm -f *.o gui.emx/app.elf gui.emx/package.info

.PHONY: all clean