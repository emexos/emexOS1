#pragma once
//#include "ulime.h"


#define WRITE    1
#define READ     2
#define EXIT     0

#define __GETPID  3 //# call_getpid;
#define __GETCWD   79

//#define __GETPID   39
//#define __GETCWD   79

// FS:
//#define FS_OPEN    2
//#define FS_CLOSE   3
//#define FS_CHDIR   80
//#define FS_MKDIR   83


#define __BRK      12

// console colors (only in supported shells)
//ansi:                \esc[color
#define A_GFX_RED     "\033[31m"
#define A_GFX_GREEN   "\033[32m"
#define A_GFX_YELLOW  "\033[33m"
#define A_GFX_BLUE    "\033[34m"
#define A_GFX_MAGENTA "\033[35m"
#define A_GFX_CYAN    "\033[36m"
#define A_GFX_WHITE   "\033[37m"
#define A_GFX_RESET   "\033[0m"
