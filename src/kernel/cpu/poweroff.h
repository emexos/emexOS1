#ifndef POWEROFF_H
#define POWEROFF_H

#define POWEROFF_SHUTDOWN 0
#define POWEROFF_REBOOT   1

void cpu_poweroff(int operation);

#endif
