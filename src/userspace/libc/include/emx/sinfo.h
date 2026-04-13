#pragma once

#include "../../../../../shared/emex.h"
#include "../../../../kernel/user/system/sysinfo.h"
#include <string.h>

#define __EMEX__          __EMEX_KERNEL
#define __EMEXF_C__       "EMEX"
#define __EMEXF_B1__      "[emex]"
#define __EMEXF_B1C__     "[EMEX]"
#define __EMEXF_S__       "emx"
#define __EMXSOPN__       __EMEX__
#define __EMXSOP_N_F_B1__ "[" __EMEX__ "]"

#define __EMX_VER_         __EMEX_VERSION
#define __EMX_BUILD_       __EMEX_BUILD
#define __EMX_BUILD_B_     "b" __EMEX_BUILD

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

typedef struct {
    char sysname[32];
    char nodename[32];
    char release[32];
    char version[32];
    char machine[32];
} emx_sinfo_t;
static inline int emx_sinfo(emx_sinfo_t *info)
{
    if (!info) return -1;

    strncpy(info->sysname, __EMEX__, sizeof(info->sysname));
    strncpy(info->nodename, "emx-host", sizeof(info->nodename));
    strncpy(info->release, __EMX_VER_, sizeof(info->release));
    strncpy(info->version, __EMX_BUILD_B_, sizeof(info->version));
    strncpy(info->machine, "x86_64", sizeof(info->machine));

    return 0;
}

/*
 * b1 == []
 * b2 == ()
 * b3 == {}
 * b4 == <>
 */