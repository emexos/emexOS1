#pragma once

#include "console.h"

// text.c
FHDR(cmd_echo);
FHDR(cmd_clear);
FHDR(cmd_font);
FHDR(cmd_fsize);

// help.c
FHDR(cmd_help);

// system.c
FHDR(cmd_modules);
FHDR(cmd_meminfo);
FHDR(cmd_memtest);
FHDR(cmd_sysinfo);
//FHDR(cmd_usb);
//FHDR(cmd_usbinfo);

//cmos.c
FHDR(cmd_cal);
FHDR(cmd_date);
FHDR(cmd_time);
FHDR(cmd_uptime);

// File System
FHDR(cmd_tree);
FHDR(cmd_cat);
FHDR(cmd_ls);
FHDR(cmd_cd);
FHDR(cmd_mkdir);
//FHDR(cmd_edit);
FHDR(cmd_touch);

// whoami.c
FHDR(cmd_whoami);

// source.c
FHDR(cmd_source);

// keymap.c
FHDR(cmd_keymap);

// poweroff.c
FHDR(cmd_poweroff);
FHDR(cmd_reboot);
FHDR(cmd_shutdown);


// view.c
FHDR(cmd_view);
