#ifndef __GEN_H
#define __GEN_H

//source: src/kernel/console/console.h
#include <kernel/graph/graphics.h>
#include <kernel/graph/fm.h>
#include <kernel/include/ports.h>
#include <kernel/communication/serial.h>
#include <drivers/ps2/keyboard/keyboard.h>
//#include "graph/uno.h"
//#include "graph/dos.h"
#include <kernel/graph/theme.h>
#include <kernel/graph/fm.h>
#include <theme/doccr.h>
#include <config/user.h>
#include <config/system.h>
#include "login/login.h"
#include <config/user_config.h>
#include <kernel/proc/scheduler.h>
#include <kernel/proc/proc_manager.h>

#include <string/string.h>

#include <kernel/module/module.h>


void _userinit();

#endif
