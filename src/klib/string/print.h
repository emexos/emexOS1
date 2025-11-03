#ifndef PRINT_H
#define PRINT_H

#include "../../../shared/types.h"

// text output functions
void putchar(char c, u32 color);
void string(const char *str, u32 color);
void print(const char *str, u32 color);
void printInt(int value, u32 color);

void reset_cursor(void);

#endif
