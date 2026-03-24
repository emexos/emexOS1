#pragma once

#include <kernel/graph/theme.h>
#include <theme/doccr.h>

#include <config/system.h>

#include <string/string.h>


// 1 == kernel hcf
// 0 == cmdline != "install"  (normal boot continues)
int installer_run(void);

/*
 * for contributing the installer, always start services after disk drivers have sucesfuly initialized
 */