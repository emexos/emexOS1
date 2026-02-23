all:
	@$(MAKE) -C libc
	@$(MAKE) -C apps/hello
	@$(MAKE) -C apps/system

clean:
	@$(MAKE) -C libc clean
	@$(MAKE) -C apps/hello clean
	@$(MAKE) -C apps/system clean

.PHONY: all clean
