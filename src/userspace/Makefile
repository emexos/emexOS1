all:
	@$(MAKE) -C libc
	@$(MAKE) -C libs/emxfb0
	@$(MAKE) -C libs/font8x12
	@$(MAKE) -C libs/wm
	@$(MAKE) -C libs/cursor

	@$(MAKE) -C apps/system
	@$(MAKE) -C apps/shell
	@$(MAKE) -C apps/login

	@$(MAKE) -C bin/echo
	@$(MAKE) -C bin/hello
	@$(MAKE) -C bin/ls
	@$(MAKE) -C bin/tree
	@$(MAKE) -C bin/cd
	@$(MAKE) -C bin/cat
	@$(MAKE) -C bin/lsblk
	@$(MAKE) -C bin/reboot
	@$(MAKE) -C bin/sinfo
	@$(MAKE) -C bin/rm
	@$(MAKE) -C bin/touch
	@$(MAKE) -C bin/uptime

clean:
	@$(MAKE) -C libc clean
	@$(MAKE) -C libs/emxfb0 clean
	@$(MAKE) -C libs/font8x12 clean
	@$(MAKE) -C libs/wm clean
	@$(MAKE) -C libs/cursor clean

	@$(MAKE) -C apps/system clean
	@$(MAKE) -C apps/shell clean
	@$(MAKE) -C apps/login clean

	@$(MAKE) -C bin/echo clean
	@$(MAKE) -C bin/hello clean
	@$(MAKE) -C bin/ls clean
	@$(MAKE) -C bin/tree clean
	@$(MAKE) -C bin/cd clean
	@$(MAKE) -C bin/cat clean
	@$(MAKE) -C bin/lsblk clean
	@$(MAKE) -C bin/reboot clean
	@$(MAKE) -C bin/sinfo clean
	@$(MAKE) -C bin/rm clean
	@$(MAKE) -C bin/touch clean
	@$(MAKE) -C bin/uptime clean

.PHONY: all clean