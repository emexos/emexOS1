#ifndef USERSPACE_H
#define USERSPACE_H

//todo:
// move to shared/syscalls/syscall.h

#define CONSOLE_READ    0 // reads input
#define CONSOLE_WRITE   1 // prints stuff to the console (console programmms)
#define EXIT           60

//#define __GETPID   39
//#define __GETCWD   79

// FS:
//#define FS_OPEN    2
//#define FS_CLOSE   3
//#define FS_CHDIR   80
//#define FS_MKDIR   83

//#define __BRK      12


//ansi:               \esc[color
#define A_GFX_RED     "\033[31m"
#define A_GFX_GREEN   "\033[32m"
#define A_GFX_YELLOW  "\033[33m"
#define A_GFX_BLUE    "\033[34m"
#define A_GFX_MAGENTA "\033[35m"
#define A_GFX_CYAN    "\033[36m"
#define A_GFX_WHITE   "\033[37m"
#define A_GFX_RESET   "\033[0m"

#endif
