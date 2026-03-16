CC      := x86_64-elf-gcc
LD      := x86_64-elf-ld
LIBC    := ../../libc
BUILD   := build

CFLAGS  := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
           -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2    \
           -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
           -I$(LIBC)/include

LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

BG_SRCS  := main.c graphics.c background.c wm.c
BG_OBJS  := $(BG_SRCS:%.c=$(BUILD)/%.o)

CUR_SRCS := cursor.c
CUR_OBJS := $(CUR_SRCS:%.c=$(BUILD)/%.o)

all: $(BUILD) bg_test.elf cursor.elf

bg_test.elf: $(BG_OBJS) $(LIBC)/build/crt0.o $(LIBC)/build/libc.a
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o $(BG_OBJS) $(LIBC)/build/libc.a -o $@

cursor.elf: $(CUR_OBJS) $(LIBC)/build/crt0.o $(LIBC)/build/libc.a
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o $(CUR_OBJS) $(LIBC)/build/libc.a -o $@

$(BUILD)/%.o: %.c gen.h font_8x12.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBC)/build/crt0.o $(LIBC)/build/libc.a:
	$(MAKE) -C $(LIBC)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) bg_test.elf cursor.elf

.PHONY: all clean