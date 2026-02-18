#ifndef PRINT_H
#define PRINT_H

#include <types.h>

void putchar_bootstrap(char c, u32 color);
void printbs(const char *str, u32 color);

// text output functions
void putchar(char c, u32 color);
void string(const char *str, u32 color);
void print(const char *str, u32 color);
void printInt(int value, u32 color);

void reset_cursor(void);

#endif
