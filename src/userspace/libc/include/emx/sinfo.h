#pragma once

#include "../../../../../shared/emex.h"
#include "../../../../kernel/user/sysinfo.h"

#define __EMEX__          __EMEX_KERNEL
#define __EMEXF_C__       "EMEX"
#define __EMEXF_B1__      "[emex]"
#define __EMEXF_B1C__     "[EMEX]"
#define __EMEXF_S__       "emx"
#define __EMXSOPN__       __EMEX__
#define __EMXSOP_N_F_B1__ "[" __EMEX__ "]"

#define __EMX_VER_         __EMEX_VERSION
#define __EMX_BUILD_       __EMEX_BUILD
#define __EMX_BUILD_B_     "b26M15"

int sysinfo(struct sysinfo_t *info);

/*
 * F_C   == fromat caps
 * F_B1  == fromat brackets type 1
 * F_B2  == fromat brackets type 2
 * F_S   == format short
 *
 * SOP == system or program
 *
 * N == name
 */

/*
 * b1 == []
 * b2 == ()
 * b3 == {}
 * b4 == <>
 */