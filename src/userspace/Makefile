all:
	@$(MAKE) -C libc

	#@$(MAKE) -C apps/hello
	@$(MAKE) -C apps/system
	@$(MAKE) -C apps/shell
	@$(MAKE) -C apps/login

	@$(MAKE) -C bin/echo
	@$(MAKE) -C bin/hello
	@$(MAKE) -C bin/ls
	@$(MAKE) -C bin/tree
	@$(MAKE) -C bin/cd
	@$(MAKE) -C bin/cat

clean:
	@$(MAKE) -C libc clean

	#@$(MAKE) -C apps/hello clean
	@$(MAKE) -C apps/system clean
	@$(MAKE) -C apps/shell clean
	@$(MAKE) -C apps/login clean

	@$(MAKE) -C bin/echo clean
	@$(MAKE) -C bin/hello clean
	@$(MAKE) -C bin/ls clean
	@$(MAKE) -C bin/tree clean
	@$(MAKE) -C bin/cd clean
	@$(MAKE) -C bin/cat clean

.PHONY: all clean
