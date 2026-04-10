CC      := x86_64-elf-gcc
LD      := x86_64-elf-ld
LIBC    := ../../libc
LIBS_D  := ../../libs

CFLAGS  := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
           -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2   \
           -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11             \
           -I$(LIBC)/include          \
           -I$(LIBS_D)/libdesktop     \
           -I$(LIBS_D)/font8x12       \
           -I$(LIBS_D)/emxfb0

LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

LIBS    := $(LIBS_D)/libdesktop/libdesktop.a  \
           $(LIBS_D)/font8x12/libfont8x12.a   \
           $(LIBS_D)/emxfb0/libemxfb0.a

all: libs clean shelly.emx/app.elf

shelly.emx/app.elf: shell.o $(LIBC)/build/crt0.o $(LIBC)/build/libc.a $(LIBS)
	mkdir -p shell.emx
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o shell.o $(LIBS) $(LIBC)/build/libc.a -o $@

shelly.o: shell.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBC)/build/crt0.o $(LIBC)/build/libc.a:
	$(MAKE) -C $(LIBC)

libs:
	$(MAKE) -C $(LIBS_D)/emxfb0
	$(MAKE) -C $(LIBS_D)/font8x12
	$(MAKE) -C $(LIBS_D)/libdesktop

clean:
	rm -f *.o shelly.emx/app.elf

.PHONY: all libs clean