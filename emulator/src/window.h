
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
	
} button_style_t;

typedef struct {
	// Window background color.
	uint32_t background;
	
	// Matrix display background color.
	uint32_t dispOff;
	// Matrix display foreground color.
	uint32_t dispOn;
	
	// Default text color.
	uint32_t text;
	// Register value color.
	uint32_t regsValue;
	// General register color.
	uint32_t regsGeneral;
	// Special register color.
	uint32_t regsSpecial;
	// Hidden register color.
	uint32_t regsHidden;
} style_t;

typedef struct {
	
} button_t;

#define DEFAULT_STYLE() ((style_t){\
	.background  = 0x2f2f2f, \
	.dispOff     = 0x3f3f3f, \
	.dispOn      = 0x4f7fff, \
	.text        = 0xefefef, \
	.regsValue   = 0x4f7fff, \
	.regsGeneral = 0xefdf9f, \
	.regsSpecial = 0xef5f00, \
	.regsHidden  = 0xafafaf, \
})

extern style_t style;

void window_init();
void window_main();
void window_destroy();
void window_redraw();
bool window_poll();

#endif // WINDOW_H
