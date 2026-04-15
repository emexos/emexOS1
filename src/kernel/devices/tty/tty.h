#ifndef TTY_H
#define TTY_H

#include <types.h>

#define TTY_COUNT 4

// echo modes
#define TTY_ECHO      0
#define TTY_NOECHO    1
#define TTY_MASKECHO  2
#define TTY_RAW       3

typedef enum {
    TTY_ANSI_NORMAL = 0,
    TTY_ANSI_ESC,
    TTY_ANSI_CSI,
} tty_ansi_state_t;

typedef struct {
    tty_ansi_state_t ansi_state;
    int ansi_param;
    u32 ansi_fg;
    u32 ansi_bg;
    int ansi_private;
    int echo_mode;
    u32 cursor_x;
    u32 cursor_y;
    int kbd_fd;
    int valid;
    int ansi_params[16];
    int ansi_param_count;
} tty_t;

void tty_init(void);
int tty_get_active(void);
void tty_set_active(int id);
tty_t  *tty_get(int id);
void tty_save_cursor(int id);
void tty_restore_cursor(int id);
void tty_write_char(int id, char c);
int tty_dev_read(int id, void *buf, size_t count);
void tty_set_echo(int id, int mode);
int tty_get_echo(int id);

#endif