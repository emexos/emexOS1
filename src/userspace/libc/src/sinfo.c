#include <emx/sinfo.h>
#include "_syscall.h"

int sysinfo(struct sysinfo_t *info)
{
	(int) _sc1(_SCAL_SYSINFO, (long) info);
}