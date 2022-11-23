
#include "window.h"
#include "debugger.h"
#include "main.h"
#include <stdarg.h>

Display  *disp;
static int       screen;
static Window    win;
static GC        gc;

style_t style = DEFAULT_STYLE();

static int mouseX =   0, mouseY =   0;
static int width  = 340, height = 400;

static bool windowOpen = true;
static const char *fontName = "8x13bold";
static XFontStruct *font;

static button_t runButton;
static button_t stepButton;
static button_t warpButton;

static bool dirty = true;

static Atom deleteWindowAtom;

void DrawText(Display *disp, Window win, GC gc, int x, int y, const char *str) {
	while (*str) {
		char *ptr = strchr(str, '\n');
		if (ptr) {
			XDrawString(disp, win, gc, x, y, str, ptr - str);
			str = ptr + 1;
			y += font->ascent + font->descent;
		} else {
			XDrawString(disp, win, gc, x, y, str, strlen(str));
			return;
		}
	}
}

void DrawTextf(Display *disp, Window win, GC gc, int x, int y, const char *fmt, ...) {
	va_list vargs;
	
	// Calculate mem requirements.
	va_start(vargs, fmt);
	int len = vsnprintf(NULL, 0, fmt, vargs);
	va_end(vargs);
	
	// Allocate memory.
	char *mem = malloc(len+1);
	if (!mem) return;
	
	// Format string.
	va_start(vargs, fmt);
	vsnprintf(mem, len+1, fmt, vargs);
	va_end(vargs);
	
	// Draw text.
	DrawText(disp, win, gc, x, y, mem);
	free(mem);
}

void CenterText(Display *disp, Window win, GC gc, int x, int y, const char *str) {
	int width = XTextWidth(font, str, strlen(str));
	DrawText(disp, win, gc, x - width / 2, y, str);
}

void CenterTextf(Display *disp, Window win, GC gc, int x, int y, const char *fmt, ...) {
	va_list vargs;
	
	// Calculate mem requirements.
	va_start(vargs, fmt);
	int len = vsnprintf(NULL, 0, fmt, vargs);
	va_end(vargs);
	
	// Allocate memory.
	char *mem = malloc(len+1);
	if (!mem) return;
	
	// Format string.
	va_start(vargs, fmt);
	vsnprintf(mem, len+1, fmt, vargs);
	va_end(vargs);
	
	// Draw text.
	CenterText(disp, win, gc, x, y, mem);
	free(mem);
}

int TextWidth(const char *text) {
	return XTextWidth(font, text, strlen(text));
}

int CalcSpacing(int elemWidth, int count, int total) {
	if (count > 0)
		return (total - elemWidth) / (count - 1);
	else
		return 0;
}



static void buttonCbPlayPause(void *ignored) {
	running           = !running;
	runButton.art     = !running ? BUTTON_ART_PLAY : BUTTON_ART_PAUSE;
	stepButton.active = !running;
}

static void buttonCbStep(void *ignored) {
	running          = false;
	sim_total_ticks += fast_tick(&cpu, &mem);
}

static void buttonCbWarp(void *ignored) {
	running           = true;
	warp_speed        = !warp_speed;
	stepButton.active = false;
	runButton.art     = running ? BUTTON_ART_PAUSE : BUTTON_ART_PLAY;
	warpButton.art    = warp_speed ? BUTTON_ART_FAST_FORWARD_END : BUTTON_ART_FAST_FORWARD;
}



void handleButtonEvent(button_t *button, XEvent event) {
	// Determine whether the mouse is over the button.
	bool hovered = mouseX >= button->x && mouseX < button->x + button->width
				&& mouseY >= button->y && mouseY < button->y + button->height;
	
	if (event.type == ButtonPress && event.xbutton.window == button->win) {
		button->pressed = hovered;
	} else if (event.type == MotionNotify && event.xmotion.window == button->win) {
		button->pressed &= hovered;
	} else if (event.type == ButtonRelease && event.xbutton.window == button->win) {
		button->pressed &= hovered;
		if (button->pressed && button->callback && button->active) {
			button->callback(button->callback_args);
		}
		button->pressed = false;
	}
}

void drawButton(button_t *button) {
	// Determine whether the mouse is over the button.
	bool hovered = mouseX >= button->x && mouseX < button->x + button->width
				&& mouseY >= button->y && mouseY < button->y + button->height;
	
	// Determine the button style to use.
	button_style_t buttonStyle  = !button->active ? style.buttons.inactive
								: button->pressed ? style.buttons.pressed
								: hovered         ? style.buttons.hovered
								: style.buttons.active;
	
	// Draw the button background.
	XSetForeground(button->disp, button->gc, buttonStyle.background);
	XFillRectangle(button->disp, button->win, button->gc, button->x, button->y, button->width, button->height);
	
	// Draw the button outline.
	XSetForeground(button->disp, button->gc, buttonStyle.border);
	XDrawLine(button->disp, button->win, button->gc, button->x, button->y, button->x + button->width - 1, button->y);
	XDrawLine(button->disp, button->win, button->gc, button->x + button->width - 1, button->y, button->x + button->width - 1, button->y + button->height - 1);
	XDrawLine(button->disp, button->win, button->gc, button->x + button->width - 1, button->y + button->height - 1, button->x, button->y + button->height - 1);
	XDrawLine(button->disp, button->win, button->gc, button->x, button->y + button->height - 1, button->x, button->y);
	
	// Draw the button text.
	XSetForeground(button->disp, button->gc, buttonStyle.foreground);
	if (button->text) {
		const char *str = button->text;
		
		int width = XTextWidth(font, str, strlen(str));
		XDrawString(
			button->disp, button->win, button->gc,
			button->x + (button->width - width) / 2,
			button->y + (button->height + font->ascent) / 2,
			str, strlen(str)
		);
	}
	
	switch (button->art) {
		case BUTTON_ART_PLAY: {
			XPoint tri[3] = {
				{ button->x + button->width / 3,     button->y + button->height / 3},
				{ button->x + button->width * 2 / 3, button->y + button->height / 2},
				{ button->x + button->width / 3,     button->y + button->height * 2 / 3},
			};
			XFillPolygon(button->disp, button->win, button->gc, tri, 3, Convex, CoordModeOrigin);
		} break;
		
		case BUTTON_ART_PAUSE: {
			XFillRectangle(
				button->disp, button->win, button->gc,
				button->x + button->width  / 3,
				button->y + button->height / 3 + 1,
				button->width  / 9,
				button->height / 3 - 1
			);
			XFillRectangle(
				button->disp, button->win, button->gc,
				button->x + button->width  * 5 / 9,
				button->y + button->height / 3 + 1,
				button->width  / 9,
				button->height / 3 - 1
			);
		} break;
		
		case BUTTON_ART_SKIP: {
			XPoint tri[3] = {
				{ button->x + button->width / 3,     button->y + button->height / 3},
				{ button->x + button->width * 5 / 9, button->y + button->height / 2},
				{ button->x + button->width / 3,     button->y + button->height * 2 / 3},
			};
			XFillPolygon(button->disp, button->win, button->gc, tri, 3, Convex, CoordModeOrigin);
			XFillRectangle(
				button->disp, button->win, button->gc,
				button->x + button->width  * 5 / 9,
				button->y + button->height / 3 + 1,
				button->width  / 9,
				button->height / 3 - 1
			);
		} break;
		
		case BUTTON_ART_FAST_FORWARD: {
			XPoint tri0[3] = {
				{ button->x + button->width * 3 / 9, button->y + button->height / 3},
				{ button->x + button->width * 5 / 9, button->y + button->height / 2},
				{ button->x + button->width * 3 / 9, button->y + button->height * 2 / 3},
			};
			XFillPolygon(button->disp, button->win, button->gc, tri0, 3, Convex, CoordModeOrigin);
			XPoint tri1[3] = {
				{ button->x + button->width * 5 / 9,     button->y + button->height / 3},
				{ button->x + button->width * 7 / 9 - 1, button->y + button->height / 2},
				{ button->x + button->width * 5 / 9,     button->y + button->height * 2 / 3},
			};
			XFillPolygon(button->disp, button->win, button->gc, tri1, 3, Convex, CoordModeOrigin);
		} break;
		
		case BUTTON_ART_FAST_FORWARD_END: {
			XPoint tri0[3] = {
				{ button->x + button->width * 2 / 9, button->y + button->height / 3},
				{ button->x + button->width * 4 / 9, button->y + button->height / 2},
				{ button->x + button->width * 2 / 9, button->y + button->height * 2 / 3},
			};
			XFillPolygon(button->disp, button->win, button->gc, tri0, 3, Convex, CoordModeOrigin);
			XPoint tri1[3] = {
				{ button->x + button->width * 4 / 9, button->y + button->height / 3},
				{ button->x + button->width * 6 / 9, button->y + button->height / 2},
				{ button->x + button->width * 4 / 9, button->y + button->height * 2 / 3},
			};
			XFillPolygon(button->disp, button->win, button->gc, tri1, 3, Convex, CoordModeOrigin);
			XFillRectangle(
				button->disp, button->win, button->gc,
				button->x + button->width  * 6 / 9,
				button->y + button->height / 3 + 1,
				button->width  / 9,
				button->height / 3 - 1
			);
		} break;
	}
}

void fmtNumber(char *buf, size_t buf_cap, double num, int len) {
	char name;
	double magnitude = num < 0 ? -num : num;
	
	if (magnitude >= 750000000000000) {
		name = 'P';
		num /= 1000000000000000;
	} else if (magnitude >= 750000000000) {
		name = 'T';
		num /= 1000000000000;
	} else if (magnitude >= 750000000) {
		name = 'G';
		num /= 1000000000;
	} else if (magnitude >= 750000) {
		name = 'M';
		num /= 1000000;
	} else if (magnitude >= 750) {
		name = 'K';
		num /= 1000;
	} else {
		snprintf(buf, buf_cap, "%*.0f ", len, num);
		return;
	}
	
	int post = 1;
	snprintf(buf, buf_cap, "%*.*f%c", len, post, num, name);
}



static void drawDisplay() {
	XSetForeground(disp, gc, style.text);
	CenterText(disp, win, gc, 170, 15, "Matrix Display");
	
	for (int x = 0; x < 32; x++) {
		word col = mem.mem_read(&cpu, &mem, 0xffc0 + x, true, mem.mem_ctx);
		for (int y = 0; y < 16; y++) {
			bool bit = (col >> y) & 1;
			XSetForeground(disp, gc, bit ? style.dispOn : style.dispOff);
			XFillRectangle(disp, win, gc, 10 + x*10, 20+y*10, 10, 10);
		}
	}
}

static void drawRegfile() {
	// Clear area behind REGISTRAR.
	XSetForeground(disp, gc, style.background);
	XFillRectangle(disp, win, gc, 10, 180, 320, 90);
	
	// HEADRE.
	XSetForeground(disp, gc, style.text);
	CenterText(disp, win, gc, 170, 200, "Registers");
	
	// Calculate spacing between the thingies.
	int spacing = CalcSpacing(TextWidth("1234"), 7, 320);
	
	// Register names.
	const char *regNames[7] = {
		"R0  ", "R1  ", "R2  ", "R3  ", "ST  ", "PF  ", "PC  ",
	};
	const char *hregNames[7] = {
		"Imm0", "Imm1", "PbA ", "PbB ", "AR  ", "Db  ", "Ab  ",
	};
	
	// Get hidden register values.
	word hregs[7] = {
		cpu.imm0, cpu.imm1, cpu.par_bus_a, cpu.par_bus_b, cpu.AR, cpu.data_bus, cpu.addr_bus
	};
	
	for (int x = 0; x < 7; x++) {
		// Register names.
		XSetForeground(disp, gc, x < 4 ? style.regsGeneral : style.regsSpecial);
		DrawText (disp, win, gc, 10 + x * spacing, 215, regNames[x]);
		// Register values.
		XSetForeground(disp, gc, style.regsValue);
		DrawTextf(disp, win, gc, 10 + x * spacing, 230, "%04X", cpu.regfile[x]);
		
		// Hidden reg names.
		XSetForeground(disp, gc, style.regsHidden);
		DrawText (disp, win, gc, 10 + x * spacing, 245, hregNames[x]);
		// Hidden reg values.
		XSetForeground(disp, gc, style.regsValue);
		DrawTextf(disp, win, gc, 10 + x * spacing, 260, "%04X", hregs[x]);
	}
}

static void drawStats() {
	char tmp[32];
	XSetForeground(disp, gc, style.text);
	CenterText(disp, win, gc, 170, 280, "Statistics");
	
	// Clear background.
	XSetForeground(disp, gc, style.background);
	XFillRectangle(disp, win, gc, 10, 295, 320, 25);
	
	
	// Draw speed.
	XSetForeground(disp, gc, style.text);
	DrawText(disp, win, gc, 10, 295, "Speed");
	
	XSetForeground(disp, gc, style.regsValue);
	fmtNumber(tmp, sizeof(tmp), measured_hertz, 5);
	DrawTextf(disp, win, gc, 10, 310, "%sHz", tmp);
	
	
	// Draw cycles.
	XSetForeground(disp, gc, style.text);
	DrawText(disp, win, gc, 105, 295, "Ticks");
	
	XSetForeground(disp, gc, style.regsValue);
	fmtNumber(tmp, sizeof(tmp), sim_total_ticks, 6);
	DrawText(disp, win, gc, 105, 310, tmp);
	
	// Draw INSN count.
	XSetForeground(disp, gc, style.text);
	DrawText(disp, win, gc, 201, 295, "Insns");
	
	XSetForeground(disp, gc, style.regsValue);
	fmtNumber(tmp, sizeof(tmp), cpu.insn_count, 6);
	DrawText(disp, win, gc, 201, 310, tmp);
	
	// Draw IPC.
	XSetForeground(disp, gc, style.text);
	DrawText(disp, win, gc, 298, 295, "IPC");
	
	XSetForeground(disp, gc, style.regsValue);
	if (sim_total_ticks) {
		double ipc = cpu.insn_count / (double) sim_total_ticks;
		DrawTextf(disp, win, gc, 298, 310, "%4.2f", ipc);
	} else {
		DrawText(disp, win, gc, 298, 310, "N/A");
	}
}



void window_init() {
	// use the information from the environment variable DISPLAY to create the X connection:
	disp   = XOpenDisplay(NULL);
	screen = DefaultScreen(disp);
	
	// Once the display is initialized, create the win.
	// This win will be have be 200 pixels across and 300 down.
	// It will have the foreground white and background black.
	win = XCreateSimpleWindow(disp, DefaultRootWindow(disp), 0, 0, width, height, 5, 0xffffff, style.background);
	
	// here is where some properties of the win can be set.
	// The third and fourth items indicate the name which appears at the top of the win and the name of the minimized win respectively.
	XSetStandardProperties(disp, win, "Pixie 16", "pixie.png", None, NULL, 0, NULL);
	
	// this routine determines which types of input are allowed in the input.
	// see the appropriate section for details...
	XSelectInput(disp, win, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask);
	
	// create the Graphics Context
	gc = XCreateGC(disp, win, 0, 0);
	
	font = XLoadQueryFont(disp, fontName);
	if (font) XSetFont(disp, gc, font->fid);
	
	// Set fixed win size.
	XSizeHints hints = (XSizeHints) {
		.flags      = PMinSize | PMaxSize,
		.min_width  = width,
		.min_height = height,
		.max_width  = width,
		.max_height = height,
	};
	XSetWMNormalHints(disp, win, &hints);
	
	// clear the win and bring it on top of the other windows
	XClearWindow(disp, win);
	XMapRaised(disp, win);
	
	// Add window closing atom.
	deleteWindowAtom = XInternAtom(disp, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(disp, win, &deleteWindowAtom, 1);
	
	debugger_init();
	debugger_show();
	
	// Create UI elements.
	runButton = (button_t) {
		.disp     = disp,
		.win      = win,
		.gc       = gc,
		
		.x        = 10,
		.y        = 345,
		.width    = 30,
		.height   = 30,
		
		.active   = true,
		.pressed  = false,
		
		.art      = BUTTON_ART_PLAY,
		
		.text     = NULL,
		.callback = buttonCbPlayPause
	};
	
	stepButton = (button_t) {
		.disp     = disp,
		.win      = win,
		.gc       = gc,
		
		.x        = 50,
		.y        = 345,
		.width    = 30,
		.height   = 30,
		
		.active   = true,
		.pressed  = false,
		
		.art      = BUTTON_ART_SKIP,
		
		.text     = NULL,
		.callback = buttonCbStep
	};
	
	warpButton = (button_t) {
		.disp     = disp,
		.win      = win,
		.gc       = gc,
		
		.x        = 90,
		.y        = 345,
		.width    = 30,
		.height   = 30,
		
		.active   = true,
		.pressed  = false,
		
		.art      = BUTTON_ART_FAST_FORWARD,
		
		.text     = NULL,
		.callback = buttonCbWarp
	};
}

void window_main() {
	while (windowOpen) {
		if (dirty || running) {
			window_redraw();
			dirty = false;
		}
		
		while (XPending(disp) && window_poll());
		
		usleep(1000000 / 60);
	}
}

void window_destroy() {
	debugger_close();
	
	if (windowOpen) {
		XUnmapWindow(disp, win);
		XDestroyWindow(disp, win);
		XFreeGC(disp, gc);
	}
	
	windowOpen = false;
}

void window_redraw() {
	// Draw display.
	drawDisplay();
	
	// Draw regs.
	drawRegfile();
	
	// Draw stats.
	drawStats();
	
	XSetForeground(disp, gc, style.text);
	CenterText(disp, win, gc, 170, 330, "Controls");
	
	// Draw UI elements.
	drawButton(&runButton);
	drawButton(&stepButton);
	drawButton(&warpButton);
	
	XFlush(disp);
}

bool window_poll() {
	XEvent msg;
	XNextEvent(disp, &msg);
	
	if (msg.type == ClientMessage) {
		if ((Atom) msg.xclient.data.l[0] == deleteWindowAtom && msg.xclient.window == win) {
			window_destroy();
			return false;
		}
	} else if (msg.type == MotionNotify && msg.xmotion.window == win) {
		mouseX = msg.xmotion.x;
		mouseY = msg.xmotion.y;
		dirty  = true;
	} else if (msg.type == ConfigureNotify && msg.xconfigure.window == win) {
		width  = msg.xconfigure.width;
		height = msg.xconfigure.height;
		dirty  = true;
	}
	
	if (debuggerOpen) debugger_event(msg);
	
	handleButtonEvent(&runButton,  msg);
	handleButtonEvent(&stepButton, msg);
	handleButtonEvent(&warpButton, msg);
	
	return true;
}
