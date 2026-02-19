AS := x86_64-elf-as
LD := x86_64-elf-ld
LDFLAGS := -nostdlib -static -no-pie -T user.ld

EMX_DIR := hello.emx

all: $(EMX_DIR)/app.elf

$(EMX_DIR)/app.elf: hello.o user.ld
	@mkdir -p $(EMX_DIR)
	$(LD) $(LDFLAGS) hello.o -o $(EMX_DIR)/app.elf
	@echo "Built $(EMX_DIR)/app.elf"

hello.o: hello.S
	$(AS) hello.S -o hello.o

clean:
	rm -f *.o $(EMX_DIR)/app.elf

.PHONY: all clean
