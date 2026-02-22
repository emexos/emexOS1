AS := x86_64-elf-as
LD := x86_64-elf-ld
LDFLAGS := -nostdlib -static -no-pie -T ../../user.ld

all: clean app.elf

app.elf: init.o
	$(LD) $(LDFLAGS) init.o -o system.emx/app.elf
	@echo "built app.elf"

init.o: init.S
	$(AS) init.S -o init.o

clean:
	rm -f *.o system.emx/app.elf

.PHONY: all clean
