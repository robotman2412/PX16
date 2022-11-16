
#include "window.h"
#include "main.h"
#include <stdarg.h>

Display  *disp;
int       screen;
Window    window;
GC        gc;

style_t style = DEFAULT_STYLE();

int mouseX =   0, mouseY =   0;
int width  = 340, height = 400;

static bool windowOpen = true;
static const char *fontName = "8x13bold";
static XFontStruct *font;

static void SetFG(uint32_t col) {
	XSetForeground(disp, gc, col);
}

static void SetBG(uint32_t col) {
	XSetBackground(disp, gc, col);
}

static void DrawText(int x, int y, const char *str) {
	while (*str) {
		char *ptr = strchr(str, '\n');
		if (ptr) {
			XDrawString(disp, window, gc, x, y, str, ptr - str);
			str = ptr + 1;
			y += font->ascent + font->descent;
		} else {
			XDrawString(disp, window, gc, x, y, str, strlen(str));
			return;
		}
	}
}

static void DrawTextf(int x, int y, const char *fmt, ...) {
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
	DrawText(x, y, mem);
	free(mem);
}

static void CenterText(int x, int y, const char *str) {
	int width = XTextWidth(font, str, strlen(str));
	DrawText(x - width / 2, y, str);
}

static int TextWidth(const char *text) {
	return XTextWidth(font, text, strlen(text));
}

static int CalcSpacing(int elemWidth, int count, int total) {
	if (count > 0)
		return (total - elemWidth) / (count - 1);
	else
		return 0;
}



static void drawDisplay() {
	SetFG(style.text);
	CenterText(170, 15, "Matrix Display");
	
	for (int x = 0; x < 32; x++) {
		word col = mem.mem_read(&cpu, &mem, 0xffc0 + x, true, mem.mem_ctx);
		for (int y = 0; y < 16; y++) {
			bool bit = (col >> y) & 1;
			XSetForeground(disp, gc, bit ? style.dispOn : style.dispOff);
			XFillRectangle(disp, window, gc, 10 + x*10, 20+y*10, 10, 10);
		}
	}
}

static void drawRegfile() {
	SetFG(style.text);
	CenterText(170, 200, "Registers");
	
	// Clear area behind REGISTRAR.
	SetFG(style.background);
	XDrawRectangle(disp, window, gc, 10, 180, 320, 90);
	
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
		SetFG(x < 4 ? style.regsGeneral : style.regsSpecial);
		DrawText (10 + x * spacing, 220, regNames[x]);
		// Register values.
		SetFG(style.regsValue);
		DrawTextf(10 + x * spacing, 230, "%04x", cpu.regfile[x]);
		
		// Hidden reg names.
		SetFG(style.regsHidden);
		DrawText (10 + x * spacing, 250, hregNames[x]);
		// Hidden reg values.
		SetFG(style.regsValue);
		DrawTextf(10 + x * spacing, 260, "%04x", hregs[x]);
	}
}



void window_init() {
	// use the information from the environment variable DISPLAY to create the X connection:
	disp   = XOpenDisplay(NULL);
	screen = DefaultScreen(disp);
	
	// Once the display is initialized, create the window.
	// This window will be have be 200 pixels across and 300 down.
	// It will have the foreground white and background black.
	window = XCreateSimpleWindow(disp, DefaultRootWindow(disp), 0, 0, width, height, 5, 0xffffff, style.background);
	
	// here is where some properties of the window can be set.
	// The third and fourth items indicate the name which appears at the top of the window and the name of the minimized window respectively.
	XSetStandardProperties(disp, window, "Pixie 16", "pixie.png", None, NULL, 0, NULL);
	
	// this routine determines which types of input are allowed in the input.
	// see the appropriate section for details...
	XSelectInput(disp, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask);
	
	// create the Graphics Context
	gc = XCreateGC(disp, window, 0, 0);
	
	font = XLoadQueryFont(disp, fontName);
	if (font) XSetFont(disp, gc, font->fid);
	
	// Set fixed window size.
	XSizeHints hints = (XSizeHints) {
		.flags      = PMinSize | PMaxSize,
		.min_width  = width,
		.min_height = height,
		.max_width  = width,
		.max_height = height,
	};
	XSetWMNormalHints(disp, window, &hints);
	
	// clear the window and bring it on top of the other windows
	XClearWindow(disp, window);
	XMapRaised(disp, window);
}

void window_main() {
	bool dirty = true;
	
	while (windowOpen) {
		if (dirty || running) {
			window_redraw();
			dirty = false;
		}
		
		while (window_poll()) {
			dirty = true;
		}
		
		usleep(1000000 / 60);
	}
}

void window_destroy() {
	
}

void window_redraw() {
	// Draw display.
	drawDisplay();
	
	// Draw regs.
	drawRegfile();
	
	XFlush(disp);
}

bool window_poll() {
	XEvent msg;
	if (XCheckWindowEvent(disp, window, -1, &msg)) {
		if (msg.type == MotionNotify) {
			mouseX = msg.xmotion.x;
			mouseY = msg.xmotion.y;
		} else if (msg.type == ConfigureNotify) {
			width  = msg.xconfigure.width;
			height = msg.xconfigure.height;
		}
		
		// Handlage.
		return true;
	}
	return false;
}
