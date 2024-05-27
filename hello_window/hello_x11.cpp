#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char* msg = "Hello, World!";

    Display* d = XOpenDisplay(NULL);
    if (d == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    int s = DefaultScreen(d);
    int screen_width = DisplayWidth(d, s);
    int screen_height = DisplayHeight(d, s);

    int win_width = 800;
    int win_height = 600;

    int win_x = (screen_width - win_width) / 2;
    int win_y = (screen_height - win_height) / 2;

    Window w = XCreateSimpleWindow(d, RootWindow(d, s), 0, 0, win_width, win_height, 1, BlackPixel(d, s), WhitePixel(d, s));
    XSelectInput(d, w, ExposureMask | KeyPressMask);
    XMapWindow(d, w);
    XMoveWindow(d, w, win_x, win_y);

    XEvent e;
    while (1) {
        XNextEvent(d, &e);
        if (e.type == Expose) {
            XFillRectangle(d, w, DefaultGC(d, s), 20, 20, 10, 10);
            XDrawString(d, w, DefaultGC(d, s), 50, 50, msg, strlen(msg));
        }
        if (e.type == KeyPress)
            break;
    }

    XCloseDisplay(d);
    return 0;
}