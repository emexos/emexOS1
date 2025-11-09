#include "keyboard.h"
#include <kernel/console/console.h>
#include <kernel/include/ports.h>

//GERMAN LAYOUT (0x81,0x80,... is from font.h so donnot change)

static const unsigned char scancode_to_ascii[128] = {
    [0x00]=0, [0x01]=0,
    [0x02]='1',[0x03]='2',[0x04]='3',[0x05]='4',[0x06]='5',
    [0x07]='6',[0x08]='7',[0x09]='8',[0x0A]='9',[0x0B]='0',
    [0x0C]=0x83,[0x0D]='\'',[0x0E]='\b',[0x0F]='\t',
    [0x10]='q',[0x11]='w',[0x12]='e',[0x13]='r',[0x14]='t',
    [0x15]='y',[0x16]='u',[0x17]='i',[0x18]='o',[0x19]='p',
    [0x1A]=0x82,[0x1B]='+',[0x1C]='\n',[0x1D]=0,
    [0x1E]='a',[0x1F]='s',[0x20]='d',[0x21]='f',[0x22]='g',
    [0x23]='h',[0x24]='j',[0x25]='k',[0x26]='l',[0x27]=0x81,
    [0x28]=0x80,[0x29]='<',[0x2A]=0,[0x2B]='#',[0x2C]='z',
    [0x2D]='x',[0x2E]='c',[0x2F]='v',[0x30]='b',[0x31]='n',
    [0x32]='m',[0x33]=',',[0x34]='.',[0x35]='-',[0x36]=0,
    [0x37]='*',[0x38]=0,[0x39]=' ',[0x3A]=0
};

static const unsigned char scancode_to_ascii_shift[128] = {
    [0x00]=0,[0x01]=0,
    [0x02]='!',[0x03]='"',[0x04]='#',[0x05]='$',[0x06]='%',
    [0x07]='&',[0x08]='/',[0x09]='(',[0x0A]=')',[0x0B]='=',
    [0x0C]='?',[0x0D]='`',[0x0E]=0,[0x0F]=0,
    [0x10]='Q',[0x11]='W',[0x12]='E',[0x13]='R',[0x14]='T',
    [0x15]='Y',[0x16]='U',[0x17]='I',[0x18]='O',[0x19]='P',
    [0x1A]=0x82,[0x1B]='*',[0x1C]='\n',[0x1D]=0,
    [0x1E]='A',[0x1F]='S',[0x20]='D',[0x21]='F',[0x22]='G',
    [0x23]='H',[0x24]='J',[0x25]='K',[0x26]='L',[0x27]=0x81,
    [0x28]=0x80,[0x29]='>',[0x2A]=0,[0x2B]='\'',[0x2C]='Z',
    [0x2D]='X',[0x2E]='C',[0x2F]='V',[0x30]='B',[0x31]='N',
    [0x32]='M',[0x33]=';',[0x34]=':',[0x35]='_',[0x36]=0,
    [0x37]='*',[0x38]=0,[0x39]=' ',[0x3A]=0
};

void keyboard_init(void) {
    //
    // DON'T call keyboard_poll() here!
}

void keyboard_poll(void) {
    int shift = 0;
    int caps = 0;

    while (1) {
        if ((inb(0x64) & 1) == 0) continue;
        unsigned char sc = inb(0x60);
        if (sc == 0) continue;

        // Handle key release
        if (sc & 0x80) {
            unsigned char make = sc & 0x7F;
            if (make == 0x2A || make == 0x36) shift = 0;
            continue;
        }

        // Handle special keys
        if (sc == 0x2A || sc == 0x36) { shift = 1; continue; }
        if (sc == 0x3A) { caps = !caps; continue; }
        if (sc == 0x0E) { console_handle_key('\b'); continue; }
        if (sc == 0x1C) { console_handle_key('\n'); continue; }

        // Convert scancode to character
        if (sc < 128) {
            unsigned char c = shift ? scancode_to_ascii_shift[sc] : scancode_to_ascii[sc];
            if (c >= 'a' && c <= 'z') {
                if ((caps && !shift) || (!caps && shift)) {
                    c = (c - 'a') + 'A';
                }
            }
            if (c) console_handle_key((char)c);
        }
    }
}

static int keyboard_module_init(void) {
    keyboard_init();
    return 0;
}

static void keyboard_module_fini(void) {
    //
}

driver_module keyboard_module = (driver_module) {
    .name = "ps2_keyboard",
    .mount = "/drivers/keyboard",
    .version = VERSION_NUM(0, 1, 0, 0),
    .init = keyboard_module_init,
    .fini = keyboard_module_fini,
    .open = NULL, // later for fs
    .read = NULL, // later for fs
    .write = NULL, // later for fs
};
