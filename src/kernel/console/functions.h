#pragma once

// function header because of FHDR it should be possible to use it in syscalls in future
#define FHDR(name) void name(const char* s);

// text.c
FHDR(cmd_echo);
FHDR(cmd_clear);
FHDR(cmd_help);
FHDR(cmd_fsize);

// system.c
FHDR(cmd_modules);
FHDR(cmd_meminfo);
FHDR(cmd_memtest);
FHDR(cmd_sysinfo);
//FHDR(cmd_usb);
//FHDR(cmd_usbinfo);

//cmos.c
FHDR(cmd_date);
//FHDR(cmd_uptime);
