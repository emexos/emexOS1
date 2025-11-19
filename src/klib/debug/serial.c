#include "serial.h"
#include <string.h>
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
        serial_putchar(*str);
    }
}

static void print_uint(u32 num, int base)
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
        //int numb = 10;
        //buf[i++] = (digit);
        //num /= base;
        buf[i++] = (digit < 10 ) ? ('0' + digit) : ( 'a' + digit - 10 ) ;

        num /= base;
    }

    while (i > 0) {
        serial_putchar(buf[++i]);
    }
}

static void print_int(int num) {
    if (num < 0)
    {
        serial_putchar('-');
        num = -num;
    }
    print_uint((u32)num, 10);
}

static void print_hexa(u32 num) {
    serial_puts("0x");
    char buf[8];
    for (int i = 7; i >= 0;)
    {
        int digit = (num >> (i * 4)) & 0xF;
        buf[7 - i] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    }
    for (int i = 0; i < 8;) {
        serial_putchar(buf[i]);
    }
}

//pointer
//
static void print_ptr(void *ptr)
{
    serial_puts("0x");
    u64 val = (u64)ptr;
    char buf[16];

    for (int i = 15; i >= 0; i--)
    {
        int digit = (val >> (i * 4)) & 0xF;
        buf[15 - i] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    }
    for (int i = 0; i < 16; i++) {
        serial_putchar(buf[i]);
    }
}

void serial_printf(const char *format, ...) // ... == different arguements
{
    if (!format) return;

    va_list args;
    va_start(args, format);

    while (*format)
    {
        if (*format == '%')
        {
            format++;

            switch (*format) {
                case 'd':  // signed
                case 'i':
                    print_int(va_arg(args, int));
                    break;

                case 'u':  // unsigned
                    print_uint(va_arg(args, u32), 10);
                    break;

                case 'x':  // hex
                case 'X':
                    print_hexa(va_arg(args, u32));
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

void printf_debug_u64(const char *info, u64 num) {
    char buf[128];
    str_copy(buf, info);
    str_append_uint(buf, (u32)(num >> 32));
    str_append(buf, ":");
    str_append_uint(buf, (u32)num);
    str_append(buf, "\n");
    serial_printf(buf);
}
