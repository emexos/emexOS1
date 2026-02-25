#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

// convert ul to ascii
static int _uitoa(unsigned long val, int base, char *buf) {
    static const char digits[] = "0123456789abcdef";
    char tmp[64];
    int  i = 0;
    if (val == 0) { tmp[i++] = '0'; }
    while (val) { tmp[i++] = digits[val % (unsigned)base]; val /= (unsigned)base; }
    for (int j = 0; j < i; j++) buf[j] = tmp[i - 1 - j];
    buf[i] = '\0';
    return i;
}


int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap) {
    size_t pos = 0;

#define _EMIT(c) do { if (buf && pos + 1 < size) buf[pos] = (c); pos++; } while(0)

    for (const char *p = fmt; *p; p++) {
        if (*p != '%') { _EMIT(*p); continue; }
        p++;

        // zero-pad flag
        int zero_pad = 0;
        if (*p == '0') { zero_pad = 1; p++; }

        // width
        int width = 0;
        while (*p >= '0' && *p <= '9') { width = width * 10 + (*p - '0'); p++; }

        // length modifier l
        int is_long = 0;
        if (*p == 'l') { is_long = 1; p++; }

        char tmp[64];
        int  tlen = 0;
        char pad  = zero_pad ? '0' : ' ';

        switch (*p) {
            case 's': {
                const char *s = va_arg(ap, const char *);
                if (!s) s = "(null)";
                tlen = (int)strlen(s);
                for (int i = tlen; i < width; i++) _EMIT(' ');
                for (int i = 0; i < tlen; i++) _EMIT(s[i]);
                tlen = 0;
                break;
            }
            case 'd': {
                long val = is_long ? va_arg(ap, long) : (long)va_arg(ap, int);
                int neg = (val < 0);
                if (neg) val = -val;
                tlen = _uitoa((unsigned long)val, 10, tmp);
                for (int i = tlen + neg; i < width; i++) _EMIT(pad);
                if (neg) _EMIT('-');
                for (int i = 0; i < tlen; i++) _EMIT(tmp[i]);
                tlen = 0;
                break;
            }
            case 'u': {
                unsigned long val = is_long ? va_arg(ap, unsigned long)
                                            : (unsigned long)va_arg(ap, unsigned int);
                tlen = _uitoa(val, 10, tmp);
                for (int i = tlen; i < width; i++) _EMIT(pad);
                for (int i = 0; i < tlen; i++) _EMIT(tmp[i]);
                tlen = 0;
                break;
            }
            case 'x': {
                unsigned long val = is_long ? va_arg(ap, unsigned long)
                                            : (unsigned long)va_arg(ap, unsigned int);
                tlen = _uitoa(val, 16, tmp);
                for (int i = tlen; i < width; i++) _EMIT(pad);
                for (int i = 0; i < tlen; i++) _EMIT(tmp[i]);
                tlen = 0;
                break;
            }
            case 'p': {
                unsigned long val = (unsigned long)va_arg(ap, void *);
                _EMIT('0'); _EMIT('x');
                tlen = _uitoa(val, 16, tmp);
                for (int i = 0; i < tlen; i++) _EMIT(tmp[i]);
                tlen = 0;
                break;
            }
            case 'c': {
                _EMIT((char)va_arg(ap, int));
                break;
            }
            case '%': {
                _EMIT('%');
                break;
            }
            default: {
                _EMIT('%');
                if (is_long) _EMIT('l');
                _EMIT(*p);
                break;
            }
        }
    }

    if (buf && size > 0) buf[pos < size ? pos : size - 1] = '\0';
    return (int)pos;

#undef _EMIT
}


int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return r;
}

int vprintf(const char *fmt, va_list ap) {
    char buf[512];
    int len = vsnprintf(buf, sizeof(buf), fmt, ap);
    // clamp to actual buffer size to avoid overrun
    if (len >= (int)sizeof(buf)) len = (int)sizeof(buf) - 1;
    write(STDOUT_FILENO, buf, (size_t)len);
    return len;
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vprintf(fmt, ap);
    va_end(ap);
    return r;
}

int putchar(int c) {
    char ch = (char)c;
    write(STDOUT_FILENO, &ch, 1);
    return (unsigned char)ch;
}

int puts(const char *s) {
    write(STDOUT_FILENO, s, strlen(s));
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}
