
#ifndef WINDOW_H
#define WINDOW_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

typedef struct {
	uint32_t background;
	uint32_t text;
	uint32_t dispOff;
	uint32_t dispOn;
} style_t;

#define DEFAULT_STYLE() ((style_t){0x2f2f2f, 0xefefef, 0x3f3f3f, 0x4f7fff})

extern style_t style;

void window_init();
void window_main();
void window_destroy();
void window_redraw();
bool window_poll();

#endif // WINDOW_H
