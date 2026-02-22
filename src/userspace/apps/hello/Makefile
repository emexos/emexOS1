AS := x86_64-elf-as
LD := x86_64-elf-ld
LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

EMX_DIR := hello.emx

all: clean hello.elf

hello.elf: hello.o
	$(LD) $(LDFLAGS) hello.o -o hello.elf
	@echo "Built hello.elf"

$(EMX_DIR)/app.elf: hello.elf
	@mkdir -p $(EMX_DIR)
	cp hello.elf $(EMX_DIR)/app.elf

hello.o: hello.S
	$(AS) hello.S -o hello.o

clean:
	rm -f *.o hello.elf $(EMX_DIR)/app.elf

.PHONY: all clean
