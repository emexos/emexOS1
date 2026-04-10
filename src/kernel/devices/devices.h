#pragma once

#include <kernel/devices/fb0/fb0.h>
#include <kernel/devices/tty/tty0.h>
#include <kernel/devices/random/urandom.h>
#include <kernel/devices/random/random.h>
#include <kernel/devices/input/kbd.h>
#include <kernel/devices/input/mouse0.h>
#include <kernel/devices/net/eth0.h>
#include <kernel/devices/null/null.h>
#include <kernel/devices/zero/zero.h>
#include <kernel/devices/disks/hdd0.h>

#define ATANAME "dev_atahdd0"
#define ATAPATH "/dev/hda"
#define ATAUNIVERSAL VERSION_NUM(0, 1, 2, 0)

#define KBDNAME "dev_ps2_keyboard0"
#define KBDPATH "/dev/input/keyboard0"
#define KBDUNIVERSAL VERSION_NUM(0, 3, 1, 0)

#define MS0NAME "dev_ps2_mouse0"
#define MS0PATH "/dev/input/mouse0"
#define MS0UNIVERSAL VERSION_NUM(0, 0, 0, 0)

#define ETH0NAME "dev_eth0"
#define ETH0PATH "/dev/net/eth0"
#define ETH0UNIVERSAL VERSION_NUM(0, 1, 0, 0)

#define EFBNAME FBN
#define FB0NAME "dev_fb0"
#define FB0PATH "/dev/fb0"
#define FB0UNIVERSAL VERSION_NUM(0, 0, 0, 0) // always 0.0.0.0

#define ZERNAME "dev_zero"
#define ZERPATH "/dev/zero"
#define ZERUNIVERSAL VERSION_NUM(0, 0, 0, 0) // always 0.0.0.0

#define NULNAME "dev_null"
#define NULPATH "/dev/null"
#define NULUNIVERSAL VERSION_NUM(0, 0, 0, 0) // always 0.0.0.0

#define TTY0NAME "dev_tty0"
#define TTY0PATH "/dev/tty0"
#define TTY0UNIVERSAL VERSION_NUM(0, 0, 1, 2)

#define URNDNAME "dev_urandom"
#define URNDPATH "/dev/urandom"
#define URNDUNIVERSAL VERSION_NUM(0, 0, 1, 0)

#define RNDNAME "dev_random"
#define RNDPATH "/dev/random"
#define RNDUNIVERSAL VERSION_NUM(0, 0, 1, 0)