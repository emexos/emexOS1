CC     := x86_64-elf-gcc
LD     := x86_64-elf-ld
LIBC   := ../../libc
LIBS_D := ../../libs

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2    \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
          -I$(LIBC)/include                                          \
          -I$(LIBS_D)/font8x12                                       \
          -I$(LIBS_D)/libbmp                                         \
          -I$(LIBS_D)/emxfb0

LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

LIBS := $(LIBS_D)/font8x12/libfont8x12.a  \
        $(LIBS_D)/libbmp/libbmp.a         \
        $(LIBS_D)/emxfb0/libemxfb0.a

OBJS := desktop.o \
        compositor/comp.o \
        cmd/cmd.o \
        bg/bg.o \
        win/win.o \
        cursor/cursor.o \
        render/render.o \
        input/input.o \

all: libs clean desktop.elf

desktop.elf: $(OBJS) $(LIBC)/build/crt0.o $(LIBC)/build/libc.a $(LIBS)
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o $(OBJS) $(LIBS) $(LIBC)/build/libc.a -o $@

desktop.o:          desktop.c           ; $(CC) $(CFLAGS) -c $< -o $@
compositor/comp.o:  compositor/comp.c   ; $(CC) $(CFLAGS) -c $< -o $@
bg/bg.o:            bg/bg.c             ; $(CC) $(CFLAGS) -c $< -o $@
win/win.o:          win/win.c           ; $(CC) $(CFLAGS) -c $< -o $@
cursor/cursor.o:    cursor/cursor.c     ; $(CC) $(CFLAGS) -c $< -o $@
render/render.o:    render/render.c     ; $(CC) $(CFLAGS) -c $< -o $@
input/input.o:      input/input.c       ; $(CC) $(CFLAGS) -c $< -o $@
cmd/cmd.o:          cmd/cmd.c           ; $(CC) $(CFLAGS) -c $< -o $@

$(LIBC)/build/crt0.o $(LIBC)/build/libc.a:
	$(MAKE) -C $(LIBC)

libs:
	$(MAKE) -C $(LIBS_D)/font8x12
	$(MAKE) -C $(LIBS_D)/libbmp
	$(MAKE) -C $(LIBS_D)/emxfb0

clean:
	rm -f *.o compositor/*.o bg/*.o win/*.o cursor/*.o render/*.o input/*.o cmd/*.o desktop.elf

.PHONY: all libs clean