#include "serial.h"
#include <klib/string/string.h>
#include <kernel/include/ports.h>
#include <stdarg.h> // for va_...

void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80); // DLAB
    outb(COM1 + 0, 0x03); //38400 baud
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7); //threshold 14 bytes
    outb(COM1 + 4, 0x0B);
}

int serial_ready(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_putchar(char c) {
    while (!serial_ready());
    outb(COM1, c);
}

void serial_puts(const char *str) {
    if (!str) return;
    while (*str) {
        serial_putchar(*str++);
    }
}

static void print_uint32(u32 num, int base)
{
    char buf[32];
    int i = 0;

    if (num == 0) {
        serial_putchar('0');
        return;
    }

    while (num > 0)
    {
        int digit = num % base;
        buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        num /= base;
    }

    while (i > 0) {
        serial_putchar(buf[--i]);
    }
}

static void print_uint64(u64 num, int base) {
    char buf[64];
    int i = 0;

    if (num == 0) {
        serial_putchar('0');
        return;
    }

    while (num > 0) {
        int digit = num % base;
        buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        num /= base;
    }

    while (i > 0) {
        serial_putchar(buf[--i]);
    }
}

static void print_int32(i32 num) {
    if (num < 0)
    {
        serial_putchar('-');
        num = -num;
    }
    print_uint32((u32)num, 10);
}

static void print_int64(i64 num) {
    if (num < 0) {
        serial_putchar('-');
        num = -num;
    }
    print_uint64((u64)num, 10);
}

static void print_hex32(u32 num) {
    serial_puts("0x");
    char buf[8];
    for (int i = 7; i >= 0; i--) {
        int digit = (num >> (i * 4)) & 0xF;
        buf[7 - i] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    }
    for (int i = 0; i < 8; i++) {
        serial_putchar(buf[i]);
    }
}

static void print_hex64(u64 num) {
    serial_puts("0x");
    //u64 val = (u64)ptr;
    char buf[16];

    for (int i = 15; i >= 0; i--)
    {
        int digit = (num >> (i * 4)) & 0xF;
        buf[15 - i] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    }
    for (int i = 0; i < 16; i++) {
        serial_putchar(buf[i]);
    }
}

static void print_ptr(void *ptr) {
    u64 val = (u64)ptr;
    print_hex64(val);
}

void serial_printf(const char *format, ...) {
    if (!format) return;

    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;

            // check for length modifiers
            int is_long = 0;
            int is_long_long = 0;

            if (*format == 'l') {
                format++;
                is_long = 1;
                if (*format == 'l') {
                    format++;
                    is_long_long = 1;
                }
            }

            switch (*format) {
                case 'd':  // signed
                case 'i':
                    if (is_long_long) {
                        print_int64(va_arg(args, i64));
                    } else if (is_long) {
                        print_int64(va_arg(args, i64));  // On 64-bit, long is 64-bit
                    } else {
                        print_int32(va_arg(args, i32));
                    }
                    break;

                case 'u':  // unsigned
                    if (is_long_long) {
                        print_uint64(va_arg(args, u64), 10);
                    } else if (is_long) {
                        print_uint64(va_arg(args, u64), 10);
                    } else {
                        print_uint32(va_arg(args, u32), 10);
                    }
                    break;

                case 'x':  // hex
                case 'X':
                    if (is_long_long) {
                        print_hex64(va_arg(args, u64));
                    } else if (is_long) {
                        print_hex64(va_arg(args, u64));
                    } else {
                        print_hex32(va_arg(args, u32));
                    }
                    break;

                case 'p':  // pointer
                    print_ptr(va_arg(args, void*));
                    break;

                case 's':  // string
                    serial_puts(va_arg(args, const char*));
                    break;

                case 'c':  // char
                    serial_putchar((char)va_arg(args, int));
                    break;

                case '%':
                    serial_putchar('%');
                    break;

                default:
                    serial_putchar('%');
                    if (is_long_long) serial_puts("ll");
                    else if (is_long) serial_putchar('l');
                    serial_putchar(*format);
                    break;
            }

        }
        else {
            serial_putchar(*format);
        }
        format++;
    }
    va_end(args);

}
