#pragma once

#define SEPERATOR "$SEPERATOR$"

#define EMRHANDLERS { \
    "bootloader",/*kernel process*/      \
    "kernel"     /*kernel process*/      \
    ,SEPERATOR,                          \
    "kernel",    /*ulime/real process*/  \
    "user",      /*ulime/real process*/  \
    "__rt"       /*ulime/real process*/  \
};
