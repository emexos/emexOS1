#pragma once

#include "ebuild.h"

#define __EMEX_VERSION_MAJOR 1
#define __EMEX_VERSION_MINOR 0

#define __EMEX_STRINGIFY(x) #x
#define __EMEX_TOSTRING(x) __EMEX_STRINGIFY(x)

#define __EMEX_VERSION __EMEX_TOSTRING(__EMEX_VERSION_MAJOR) "." __EMEX_TOSTRING(__EMEX_VERSION_MINOR)
#define __EMEX_VERSION_V "v" __EMEX_VERSION
#define __EMEX_KERNEL "emex"
#define __EMEX_BUILD ___EMEX_BUILD

#define __EMEX_FAT32 "EMEXOS"
#define __EMEX_FAT32OEM "EMEXOS"