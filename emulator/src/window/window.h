
#ifndef WINDOW_H
#define WINDOW_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>



typedef void (*button_cb_t)(void *args);

typedef enum {
	BUTTON_ART_NONE,
	BUTTON_ART_PLAY,
	BUTTON_ART_PAUSE,
	BUTTON_ART_SKIP,
	BUTTON_ART_FAST_FORWARD,
	BUTTON_ART_FAST_FORWARD_END,
} button_art_t;



typedef struct {
	// Button background color.
	uint32_t background;
	// Button border color.
	uint32_t border;
	// Button art / text color.
	uint32_t foreground;
} button_style_t;

typedef struct {
	// Inactive button.
	button_style_t inactive;
	// Active button.
	button_style_t active;
	// Hovered button.
	button_style_t hovered;
	// Pressed button.
	button_style_t pressed;
} button_styles_t;

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
	
	// RAM background.
	uint32_t memoryRAM;
	// ROM background.
	uint32_t memoryROM;
	// MMIO background.
	uint32_t memoryMMIO;
	// VRAM background.
	uint32_t memoryVRAM;
	// Memory highlight outline color.
	uint32_t memorySel;
	// Memory text color.
	uint32_t memoryText;
	// Memory address text color.
	uint32_t memoryAddr;
	
	// Default style for buttons.
	button_styles_t buttons;
} style_t;

extern const char *style_names[];
extern const size_t n_style_names;

typedef struct {
	// Button position.
	int          x, y;
	// Button size.
	int          width, height;
	
	// Whether the button is active.
	bool         active;
	// Whether a button press has started.
	bool         pressed;
	
	// The art to put on this button.
	button_art_t art;
	// The text to put on this button, if any.
	const char  *text;
	
	// Callback for when button is pressed.
	button_cb_t  callback;
	// Context for button callback.
	void        *callback_args;
} button_t;

#define DEFAULT_BUTTON_STYLES() { \
	.inactive = { 0x2f2f2f, 0x7f7f7f, 0x7f7f7f, }, \
	.active   = { 0x2f2f2f, 0xefefef, 0xefefef, }, \
	.hovered  = { 0x3f3f3f, 0xcfcfff, 0xcfcfff, }, \
	.pressed  = { 0x000000, 0x7f7fcf, 0x7f7fcf, }, \
}

#define DEFAULT_STYLE() (style_t) { \
	.background  = 0x2f2f2f, \
	.dispOff     = 0x3f3f3f, \
	.dispOn      = 0x4f7fff, \
	.text        = 0xefefef, \
	.regsValue   = 0x4f7fff, \
	.regsGeneral = 0xefdf9f, \
	.regsSpecial = 0xef5f00, \
	.regsHidden  = 0xafafaf, \
	.memoryRAM   = 0x3f3f3f, \
	.memoryROM   = 0x5f2f2f, \
	.memoryMMIO  = 0x5f5f2f, \
	.memoryVRAM  = 0x5f5f2f, \
	.memorySel   = 0xcfcfff, \
	.memoryText  = 0xcfcfcf, \
	.memoryAddr  = 0xafafaf, \
	.buttons     = DEFAULT_BUTTON_STYLES(), \
}

extern style_t style;

void window_main();

#endif // WINDOW_H
