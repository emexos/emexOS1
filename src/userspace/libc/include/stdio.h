#pragma once
#include <stddef.h>
#include <stdarg.h>

#define EOF (-1)

int puts(const char *s);
int putchar(int c);

int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
