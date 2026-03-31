#pragma once

// use | for combining
#define DT_WIN      0x00  // n.window; titlebar; moveable
#define DT_POPUP    0x01  // a popup window with no title bar and no close button (closing needs to be handled extern)
#define DT_NOMOVE   0x02  // has titlebar but cant be dragged
#define DT_NOTITLE  0x04  // no titlebar (thats why drag is there disabled)

// example:
// ( DT_NOMOVE | DT_NOTITLE )
// is a unmoveable box; which is useful for panels

// needs to mach desktop.c/.h
// the lib cant use desktop.h/.c
#define DT_TITLE_H 18
#define DT_BORDER  1

typedef struct {
    int x, y, w, h;
} DesktopArea;

// computes the content area for a window
// passes to desktop.createWindow
static inline DesktopArea desktopContentArea(
    int x, int y, int w, int h, unsigned int style)
{
    DesktopArea a;
    if (style & DT_POPUP) {
        // 1px border all around
        a.x = x + 1;
        a.y = y + 1;
        a.w = w - 2;
        a.h = h - 2;
    } else if (style & DT_NOTITLE) {
        // 2px border
        a.x = x + DT_BORDER;
        a.y = y + DT_BORDER;
        a.w = w - DT_BORDER * 2;
        a.h = h - DT_BORDER * 2;
    } else {
        // 2px border + 18px titlebar + 1px separator
        a.x = x + DT_BORDER;
        a.y = y + DT_TITLE_H + 1;
        a.w = w - DT_BORDER * 2;
        a.h = h - DT_TITLE_H - 1 - DT_BORDER;
    }
    return a;
}

//the struct creates desktop. so you can use:
// desktop.createWindow("title", x, y, w, h, DT_WIN);
typedef struct {

	// register
    int  (*createWindow)(
    	const char *title,
        int x, int y, int w, int h,
        unsigned int style
    );

    // unregister
    void (*closeWindow)(void);
    // update title
    void (*setTitle)(const char *title);


    void (*markDirty)(void);

    // cursor position
    int  (*getCursor)(int *x, int *y);
} Desktop;

extern Desktop desktop; // desktop.