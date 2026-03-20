#pragma once

#include "../kernel/gen.h"
#include <string/string.h>

#define SYSTEMLOCATE2 "/emr/system/system.emx"
#define EPROMPT "/emergency_shell # "
#define RPROMPT "/recovery_shell # "
#define SHELL_FG_COLOR white()

void emergency_shell(void);
void recovery_shell(void);