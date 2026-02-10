#include "string.h"
#include "print.h"

void str_copy(char *dest, const char *src)
{
    int i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void str_append(char *dest, const char *src)
{
    int i = 0;
    while (dest[i] != '\0')
    {
        i++;
    }

    int j = 0;
    while (src[j] != '\0')
    {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = '\0';
}

void str_append_uint(char *dest, u32 num)
{
    char buf[20];
    int i = 0;

    if (num == 0)
    {
        buf[i++] = '0';
    }
    else
    {
        u32 temp = num;
        int digits = 0;
        while (temp > 0)
        {
            temp /= 10;
            digits++;
        }

        i = digits;
        while (num > 0)
        {
            buf[--i] = '0' + (num % 10);
            num /= 10;
        }
        i = digits;
    }

    buf[i] = '\0';
    str_append(dest, buf);
}

int str_len(const char *str)
{
    int len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    return len;
}

int str_starts_with(const char *str, const char *prefix)
{
    if (!str || !prefix) return 0;

    while (*prefix) {
        if (*str != *prefix) {
            return 0;
        }
        str++;
        prefix++;
    }
    return 1;
}

int str_equals(const char *s1, const char *s2)
{
    if (!s1 || !s2) return 0;

    while (*s1 && *s2) {
        if (*s1 != *s2) return 0;
        s1++;
        s2++;
    }
    return (*s1 == *s2);
}

void str_to_upper(char *str)
{
    if (!str) return;

    while (*str) {
        if (*str >= 'a' && *str <= 'z') {
            *str = *str - 32;
        }
        str++;
    }
}

void str_to_lower(char *str)
{
    if (!str) return;

    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            *str = *str + 32;
        }
        str++;
    }
}

void str_trim(char *str)
{
    if (!str) return;

    // Trim leading spaces
    char *start = str;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    // Trim trailing spaces
    char *end = start;
    while (*end) end++;
    end--;

    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }

    // Copy trimmed string
    int i = 0;
    while (start <= end) {
        str[i++] = *start++;
    }
    str[i] = '\0';
}

/*int str_split(const char *str, char delimiter, char **tokens, int max_tokens) {
    int count = 0;

    return count;
}*/

void str_reverse(char *str)
{
    if (!str) return;

    int len = str_len(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

int str_to_int(const char *str)
{
    if (!str) return 0;

    int result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t') str++;

    // Check sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Convert digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result * sign;
}

void str_from_hex(char *buf, u64 value)
{
    char hexchars[] = "0123456789ABCDEF";
    char tmp[32];
    int i = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return;
    }

    while (value > 0) {
        tmp[i++] = hexchars[value & 0xF];
        value >>= 4;
    }

    int j = 0;
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = 0;
}

void str_from_int(char *buf, int value)
{
    if (!buf) return;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    int is_negative = 0;
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    int i = 0;
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    buf[i] = '\0';
    str_reverse(buf);
}

int str_contains(const char *str, const char *substr)
{
    if (!str || !substr) return 0;

    int str_len = 0;
    while (str[str_len]) str_len++;

    int substr_len = 0;
    while (substr[substr_len]) substr_len++;

    if (substr_len > str_len) return 0;

    for (int i = 0; i <= str_len - substr_len; i++)
    {
        int match = 1;
        for (int j = 0; j < substr_len; j++)
        {
            if (str[i + j] != substr[j])
            {
                match = 0;
                break;
            }
        }
        if (match) return 1;
    }
    return 0;
}


void print_str(const char *str, u32 color){
    string(str, color);
    putchar('\n', color);
}
