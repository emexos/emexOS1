CC  := x86_64-elf-gcc
AR  := x86_64-elf-ar
INC := $(CURDIR)/include
BLD := $(CURDIR)/build

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2   \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11             \
          -I$(INC) -Isrc

SRCS := src/errno.c src/string.c src/stdlib.c src/unistd.c src/fcntl.c src/stdio.c
OBJS := $(patsubst src/%.c, $(BLD)/%.o, $(SRCS))

all: $(BLD)/libc.a $(BLD)/crt0.o

$(BLD)/libc.a: $(OBJS)
	$(AR) rcs $@ $^

$(BLD)/%.o: src/%.c | $(BLD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BLD)/crt0.o: src/crt0.S | $(BLD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BLD):
	mkdir -p $@

clean:
	rm -rf $(BLD)

.PHONY: all clean
