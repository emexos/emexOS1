all:
	@$(MAKE) -C libc
	@$(MAKE) -C libs/emxfb0
	@$(MAKE) -C libs/font8x12
	@$(MAKE) -C libs/libdesktop
#@$(MAKE) -C libs/cursor

	@$(MAKE) -C apps/system
		@$(MAKE) -C apps/system/stinf
	@$(MAKE) -C apps/shell
	@$(MAKE) -C apps/guishell
	@$(MAKE) -C apps/login
	@$(MAKE) -C apps/desktop

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
	@$(MAKE) -C bin/uname
	@$(MAKE) -C bin/initd
#@$(MAKE) -C bin/chaosd
	@$(MAKE) -C bin/ps
	@$(MAKE) -C bin/pwd
	@$(MAKE) -C bin/meminfo
	@$(MAKE) -C bin/dofetch

clean:
	@$(MAKE) -C libc clean
	@$(MAKE) -C libs/emxfb0 clean
	@$(MAKE) -C libs/font8x12 clean
	@$(MAKE) -C libs/libdesktop clean
#@$(MAKE) -C libs/cursor clean

	@$(MAKE) -C apps/system clean
		@$(MAKE) -C apps/system/stinf clean
	@$(MAKE) -C apps/shell clean
	@$(MAKE) -C apps/guishell clean
	@$(MAKE) -C apps/login clean
	@$(MAKE) -C apps/desktop clean

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
	@$(MAKE) -C bin/uname clean
	@$(MAKE) -C bin/initd clean
#@$(MAKE) -C bin/chaosd clean
	@$(MAKE) -C bin/ps clean
	@$(MAKE) -C bin/pwd clean
	@$(MAKE) -C bin/meminfo clean
	@$(MAKE) -C bin/dofetch clean

.PHONY: all clean