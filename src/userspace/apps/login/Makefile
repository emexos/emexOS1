CC      := x86_64-elf-gcc
LD      := x86_64-elf-ld
LIBC    := ../../libc
LIBS_D  := ../../libs

CFLAGS  := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
           -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2   \
           -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
           -I$(LIBC)/include        \
           -I$(LIBS_D)/wm           \
           -I$(LIBS_D)/emxfb0

LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

LIBS    := $(LIBS_D)/wm/libwm.a           \
           $(LIBS_D)/font8x12/libfont8x12.a \
           $(LIBS_D)/emxfb0/libemxfb0.a

all: libs login.elf

libs:
	$(MAKE) -C $(LIBS_D)/emxfb0
	$(MAKE) -C $(LIBS_D)/font8x12
	$(MAKE) -C $(LIBS_D)/wm

login.elf: login.o $(LIBC)/build/crt0.o $(LIBC)/build/libc.a $(LIBS)
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o login.o $(LIBS) $(LIBC)/build/libc.a -o $@

login.o: login.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBC)/build/crt0.o $(LIBC)/build/libc.a:
	$(MAKE) -C $(LIBC)

clean:
	rm -f *.o login.elf

.PHONY: all libs clean