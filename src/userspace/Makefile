all:
	@$(MAKE) -C libc
	#@$(MAKE) -C apps/hello
	@$(MAKE) -C apps/system
	@$(MAKE) -C apps/shell
	@$(MAKE) -C bin/echo
	@$(MAKE) -C bin/hello

clean:
	@$(MAKE) -C libc clean
	#@$(MAKE) -C apps/hello clean
	@$(MAKE) -C apps/system clean
	@$(MAKE) -C apps/shell clean
	@$(MAKE) -C bin/echo clean
	@$(MAKE) -C bin/hello clean

.PHONY: all clean
