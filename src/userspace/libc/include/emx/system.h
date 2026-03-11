#pragma once

// from linux
#define RSYSTEM_MAGIC1 0xfee1deadUL
#define RSYSTEM_MAGIC2 0x28121969UL
#define RSYSTEM_CMD_RESTART 0x01234567UL
#define RSYSTEM_CMD_HALT 0xcdef0123UL
#define RSYSTEM_CMD_POWEROFF 0x4321fedcUL

int reboot(unsigned long cmd);