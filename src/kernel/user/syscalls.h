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

#define red A_GFX_RED
#define green A_GFX_GREEN
#define yellow A_GFX_YELLOW
#define blue A_GFX_BLUE
#define magenta A_GFX_MAGENTA
#define cyan A_GFX_CYAN
#define white A_GFX_WHITE
#define reset A_GFX_RESET
