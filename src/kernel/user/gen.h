#ifndef __GEN_H
#define __GEN_H

//source: src/kernel/console/console.h
#include <kernel/graph/graphics.h>
#include <kernel/kernel_processes/fm/fm.h>
#include <kernel/kernel_processes/loader.h>
#include <kernel/include/ports.h>
#include <kernel/communication/serial.h>
#include <drivers/ps2/keyboard/keyboard.h>
//#include "graph/uno.h"
//#include "graph/dos.h"
#include <kernel/graph/theme.h>
#include <theme/doccr.h>
#include <config/user.h>
#include <config/system.h>
#include "login/login.h"
#include <config/user_config.h>
#include <kernel/proc/scheduler.h>
#include <kernel/proc/proc_manager.h>
#include <kernel/include/assembly.h>
#include <kernel/console/console.h>

#include <string/string.h>

#include <kernel/module/module.h>


void DEinit();

#endif
