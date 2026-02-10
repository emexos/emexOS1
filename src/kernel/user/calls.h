#ifndef __CALLS
#define __CALLS 1

#define X86_64 1
#define ARM64 0


#if X86_64 == 1
    #define EXIT    60

    #define WRITE    1
    #define WRITEV  20
    #define READ     0
    #define READV   19

    //#define __GETPID  391
    #define GETPID    39 //# call_getpid;
    //#define __GETCWD  791
    #define GETCWD    79

    #define GETPPID 110
    #define GETTID      186

    #define BRK 12
    #define KILL 62

    #define OPEN 2
    #define CLOSE 3

    #define STAT 4
    //#define FSTAT       5
    //#define LSTAT       6


    #define FORK        57
    #define VFORK       58
    #define EXECVE      59

    #define CHDIR       80
    #define MKDIR       83
    #define FCHDIR     133

#elif ARM64 == 1
    #define EXIT       93

    #define WRITE      64
    #define WRITEV     66
    #define READ       63
    #define READV      65

    #define GETPID    172
    #define GETCWD     17
    #define GETPPID   173
    #define GETTID    178

    #define BRK       214
    #define KILL      129

    #define OPEN       56   // openat in aarch64
    #define CLOSE      57

    #define STAT       80
    //#define FSTAT    79
    //#define LSTAT    79

    #define FORK      220
    #define VFORK     221
    #define EXECVE    221

    #define CHDIR      49
    #define MKDIR      80
    #define FCHDIR     50

#endif



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



#endif
