#pragma once

// esh will automatically load the shell after everything is finished
#define EMX_SHELL "/user/apps/shell.emx"
#define LOGINLOCATE "/emr/system/login.elf"//user/apps/login.emx"


#define SEPERATOR "$SEPERATOR$"

#define EMRHANDLERS { \
    "bootloader",/*kernel process*/      \
    "kernel"     /*kernel process*/      \
    ,SEPERATOR,                          \
    "kernel",    /*ulime/real process*/  \
    "user",      /*ulime/real process*/  \
    "__rt"       /*ulime/real process*/  \
};
