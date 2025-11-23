#include "glime.h"

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

keyboardrb_t *glime_keyboard_init(glime_t *glime, u64 count)  {
    if (!glime || count == 0) return NULL;

    //@TODO: slab_alloc
    keyboardrb_t *kbrb = (keyboardrb_t *) glime_create(glime, sizeof(keyboardrb_t));
    if (!kbrb) return NULL;

    kbrb->buf = (key_event_t *) glime_alloc(glime, sizeof(key_event_t), count);
    if (!kbrb->buf) {
        glime_free(glime, (u64 *) kbrb);
        return NULL;
    }
    
    kbrb->len = count; 
    kbrb->head = 0;
    kbrb->tail = 0;
    kbrb->count = 0;

    return kbrb;
}

int keyboard_put(keyboardrb_t *kbrb, key_event_t event) {
    if (kbrb->count >= kbrb->len) {
        return 1;
    }

    kbrb->buf[kbrb->head] = event;
    kbrb->head = (kbrb->head + 1) % kbrb->len;
    kbrb->count++;
  
    return 0;
}

int keyboard_next(keyboardrb_t *kbrb, key_event_t *out) {
    if (kbrb->count = 0) return 1;

    *out = kbrb->buf[kbrb->tail];
    kbrb->tail = (kbrb->tail + 1) % kbrb->len;
    kbrb->count--;

    return 0;
}

u8 keyboard_event_to_char(key_event_t event) {
    return scancode_to_ascii[event.scancode];
}

