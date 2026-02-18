#include "log.h"
#include <string/string.h>
#include <kernel/graph/graphics.h>
#include <kernel/communication/serial.h>
#include <stdarg.h>
#include <theme/stdclrs.h>

static u32 get_log_color(log_level_t level) {
    switch (level) {
        case LSUCCESS:
            return LCOLOR_SCS;
        case LWARNING:
            return LCOLOR_WAR;
        case LERROR:
            return LCOLOR_ERR;
        case LDEF:
        default:
            return LCOLOR_D;
    }
}


void log_message(const char *tag, const char *message, log_level_t level) {
    if (!tag || !message) return;

    u32 msg_color = get_log_color(level);

    // gray
    //print("[", LCOLOR_BRACKET);
    print(tag, LCOLOR_TAG);
    print(" ", LCOLOR_BRACKET);

    print(message, msg_color);
    //printf("%s %s", tag, message);
}
void log_printf(log_level_t level, const char *tag, const char *format, ...) {
    if (!tag || !format) return;

    u32 msg_color = get_log_color(level);

    // gray
    //print("[", LCOLOR_BRACKET);
    print(tag, LCOLOR_TAG);
    print(" ", LCOLOR_BRACKET);



    va_list args;
    va_start(args, format);
    char buffer[512];
    int pos = 0;

    // formated string
    for (const char *p = format; *p && pos < 511; p++) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 's': {
                    const char *s = va_arg(args, const char*);
                    if (s) {
                        while (*s && pos < 511) {
                            buffer[pos++] = *s++;
                        }
                    }
                    break;
                }
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    char num_buf[32];
                    str_from_int(num_buf, val);
                    for (char *n = num_buf; *n && pos < 511; n++) {
                        buffer[pos++] = *n;
                    }
                    break;
                }
                case 'u': {
                    u32 val = va_arg(args, u32);
                    char num_buf[32];
                    num_buf[0] = '\0';
                    str_append_uint(num_buf, val);
                    for (char *n = num_buf; *n && pos < 511; n++) {
                        buffer[pos++] = *n;
                    }
                    break;
                }
                case 'x':
                case 'X': {
                    u32 val = va_arg(args, u32);
                    char hex_buf[16];
                    int hex_pos = 0;
                    if (val == 0) {
                        hex_buf[hex_pos++] = '0';
                    } else {
                        u32 temp = val;
                        while (temp > 0 && hex_pos < 15) {
                            int digit = temp % 16;
                            hex_buf[hex_pos++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
                            temp /= 16;
                        }
                    }
                    hex_buf[hex_pos] = '\0';
                    // reverse hex string
                    for (int i = hex_pos - 1; i >= 0 && pos < 511; i--) {
                        buffer[pos++] = hex_buf[i];
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    if (pos < 511) {
                        buffer[pos++] = c;
                    }
                    break;
                }
                case '%': {
                    if (pos < 511) {
                        buffer[pos++] = '%';
                    }
                    break;
                }
                default:
                    if (pos < 511) {
                        buffer[pos++] = '%';
                    }
                    if (pos < 511) {
                        buffer[pos++] = *p;
                    }
                    break;
            }
        } else {
            buffer[pos++] = *p;
        }
    }

    buffer[pos] = '\0';
    va_end(args);



    print(buffer, msg_color);
    //printf("%s", buffer);
}
