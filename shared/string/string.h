#ifndef STRING_H
#define STRING_H

#include <types.h>
#include "print.h"
#include "log.h"
#include <theme/stdclrs.h>
#include <theme/doccr.h>

void str_copy(char *dest, const char *src);
void str_append(char *dest, const char *src);
void str_append_uint(char *dest, u32 num);
int str_len(const char *str);
void print_str(const char *str, u32 color);
int str_starts_with(const char *str, const char *prefix);
int str_equals(const char *s1, const char *s2);
int str_contains(const char *str, const char *substr);
void str_to_upper(char *str);

void str_append_char(char *dest, char c);


void str_from_int(char *buf, int value);
void str_from_hex(char *buf, u64 value);

#endif
