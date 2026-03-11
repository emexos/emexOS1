#include "mouse.h"
#include <kernel/include/ports.h>
#include <kernel/arch/x86_64/exceptions/irq.h>
#include <kernel/graph/graphics.h>

static mouse_event_t evbuf[MOUSE_BUF_SIZE];
static volatile int  ev_r = 0, ev_w = 0;
static void ev_push(mouse_event_t *e) {
    int next = (ev_w + 1) % MOUSE_BUF_SIZE;
    if (next != ev_r) { evbuf[ev_w] = *e; ev_w = next; }
}

int mouse_has_event(void) { return ev_r != ev_w; }
int mouse_get_event(mouse_event_t *e) {
    if (ev_r == ev_w) return 0;
    *e = evbuf[ev_r]; ev_r = (ev_r + 1) % MOUSE_BUF_SIZE;
    return 1;
}

static int abs_x = 0, abs_y = 0;
static void ps2_wait_w(void) {
	int t=100000; while(t-- && ( inb(PS2_CMD)&0x02));
}
static void ps2_wait_r(void) {
	int t=100000; while(t-- && (!inb(PS2_CMD)&0x01));
}
static u8 ps2_rd(void){
	ps2_wait_r(); return inb(PS2_DATA);
}
static void ps2_cmd(u8 c){
	ps2_wait_w(); outb(PS2_CMD, c);
}
static void ps2_dat(u8 d){
	ps2_wait_w(); outb(PS2_DATA, d);
}

static u8  pkt[3];
static int pkt_i = 0;

static void mouse_irq(cpu_state_t *s) {
    (void)s;
    u8 status = inb(PS2_CMD);
    if (!(status & 0x01)) return;

    if (!(status & 0x20)) { inb(PS2_DATA); return; }

    u8 b = inb(PS2_DATA);

    if (pkt_i == 0 && !(b & 0x08)) return; // sync check
    pkt[pkt_i++] = b;
    if (pkt_i < 3) return;
    pkt_i = 0;

    if (pkt[0] & 0xC0) return; // overflow and discard

    // 9-bit signed via sign bits in byte 0
    int dx =  (int)pkt[1] - ((pkt[0] & 0x10) ? 256 : 0);
    int dy = -((int)pkt[2] - ((pkt[0] & 0x20) ? 256 : 0));

    u32 fw = get_fb_width(), fh = get_fb_height();
    if (!fw || !fh) return;

    abs_x += dx; abs_y += dy;
    if (abs_x < 0) abs_x = 0;
    if (abs_y < 0) abs_y = 0;
    if (abs_x >= (int)fw) abs_x = (int)fw - 1;
    if (abs_y >= (int)fh) abs_y = (int)fh - 1;

    mouse_event_t ev = {
        .dx = dx, .dy = dy,
        .abs_x = abs_x, .abs_y = abs_y,
        .buttons = pkt[0] & 0x07
    };
    ev_push(&ev);
}

void mouse_init(void) {
    ps2_cmd(0xA8);
    ps2_cmd(0x20); u8 cfg = ps2_rd(); cfg |= 0x02; cfg &= ~0x20;
    ps2_cmd(0x60); ps2_dat(cfg);
    ps2_cmd(0xD4); ps2_dat(0xF6); ps2_rd();  // defaults
    ps2_cmd(0xD4); ps2_dat(0xE8); ps2_rd();  // set resolution
    ps2_cmd(0xD4); ps2_dat(0x02); ps2_rd();  // 4 counts/mm
    ps2_cmd(0xD4); ps2_dat(0xF3); ps2_rd();  // set sample rate
    ps2_cmd(0xD4); ps2_dat(40);   ps2_rd();  // 40 samples per sec
    ps2_cmd(0xD4); ps2_dat(0xF4); ps2_rd();  // enable

    abs_x = (int)(get_fb_width() / 2);
    abs_y = (int)(get_fb_height() / 2);
    pkt_i = 0;

    irq_register_handler(12, mouse_irq);
}