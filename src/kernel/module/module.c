#include "module.h"
#include <string/string.h>
#include <theme/stdclrs.h>
#include <kernel/graph/theme.h>
#include <theme/doccr.h>

static driver_module *modules[MAX_MODULES];
static int module_count = 0;

void module_init(void)
{
    log("[MOD]", "init module system\n", d);
    for (int i = 0; i < MAX_MODULES; i++)
    {
        modules[i] = NULL;
    }
    module_count = 0;
}

int module_register(driver_module *module)
{
	if (!module || module_count >= MAX_MODULES)
	{
        log("[MOD]", "Module register failed: invalid module or max reached\n", warning);
        return -1;
    }

    // check if already registered
    for (int i = 0; i < module_count; i++)
    {
        if (modules[i] == module) {
            log("[MOD]", "Module already registered\n", warning);
            return -1;
        }
    }

    // call init if exists
    if (module->init)
    {
        int ret = module->init();
        if (ret != 0) {
            log("[MOD]", "Module init failed, skipping registration\n", warning);
            return -1; /* don't add module if init fails */
        }
    }

    modules[module_count++] = module;
    return 0;
}

void module_unregister(const char *name)
{
    if (!name) return;

    for (int i = 0; i < module_count; i++)
    {
	    if (modules[i] && modules[i]->name && str_equals(modules[i]->name, name))
		{
	        // call cleanup if exists
	        if (modules[i]->fini) {
	            modules[i]->fini();
	        }

	        // shift array down
	        for (int j = i; j < module_count - 1; j++) {
	            modules[j] = modules[j + 1];
	        }
	        modules[module_count - 1] = NULL;
	        module_count--;

	        log("[MOD]", "Module unregistered\n", d);
	        return;
	    }
    }
    log("[MOD]", "Module to unregister not found\n", warning);
}

driver_module* module_find(const char *name)
{
    if (!name) return NULL;

    for (int i = 0; i < module_count; i++)
    {
        if (modules[i] && modules[i]->name && str_equals(modules[i]->name, name)) {
            return modules[i];
        }
    }
    return NULL;
}

int module_get_count(void) {
    return module_count;
}

driver_module* module_get_by_index(int idx) {
    if (idx < 0 || idx >= module_count) {
        return NULL;
    }
    return modules[idx];
}
