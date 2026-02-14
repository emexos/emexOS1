AS := x86_64-elf-as
LD := x86_64-elf-ld
LDFLAGS := -nostdlib -static -no-pie -T user.ld

all: hello.elf

hello.elf: hello.o user.ld
	$(LD) $(LDFLAGS) hello.o -o hello.elf
	@echo "Built hello.elf"

hello.o: hello.S
	$(AS) hello.S -o hello.o

clean:
	rm -f *.o hello.elf

.PHONY: all clean
