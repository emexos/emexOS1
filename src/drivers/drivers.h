#pragma once

#include <drivers/cmos/cmos.h>
#include <drivers/ps2/ps2.h>
#include <drivers/storage/ata/disk.h>
//#include <drivers/sound/sb16/sound.h>
//#include <drivers/network/gen.h>

#include <kernel/devices/fb0/fb0.h>

//ddir == dev dir

#define ATANAME "dev_atahdd0"
#define ATAPATH "/dev/hdd0"
#define ATAUNIVERSAL VERSION_NUM(0, 1, 2, 0)

#define KBDNAME "dev_ps2_keyboard0"
#define KBDPATH "/dev/input/keyboard0"
#define KBDUNIVERSAL VERSION_NUM(0, 3, 1, 0)

#define MS0NAME "dev_ps2_mouse0"
#define MS0PATH "/dev/input/mouse0"
#define MS0UNIVERSAL VERSION_NUM(0, 0, 0, 0)

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