#ifndef STRING_H
#define STRING_H

#include "../../../shared/types.h"
#include "../graphics/graphics.h"

void str_copy(char *dest, const char *src);
void str_append(char *dest, const char *src);
void str_append_uint(char *dest, u32 num);
int str_len(const char *str);
void print_str(const char *str, u32 color);

#endif
