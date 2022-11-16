
#include "window.h"
#include "main.h"

Display  *disp;
int       screen;
Window    window;
GC        gc;

style_t style = DEFAULT_STYLE();

int mouseX =   0, mouseY =   0;
int width  = 340, height = 200;

static bool windowOpen = true;
static const char *fontName = "-sony-fixed-medium-r-normal--*-*-*-*-c-0-*-*";
static XFontStruct *font;

static void SetFG(uint32_t col) {
	XSetForeground(disp, gc, col);
}

static void DrawText(int x, int y, const char *str) {
	XDrawString(disp, window, gc, x, y, str, strlen(str));
}

static void CenterText(int x, int y, const char *str) {
	// XGCValues values;
	// int qu=XGetGCValues(disp, gc, GCFont, &values);
	// printf("WTF IS %d\n",qu);
	// XFontStruct *font = XQueryFont(disp, values.font);
	if (font) {
		int width = XTextWidth(font, str, strlen(str));
		DrawText(x - width / 2, y, str);
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
	
	// clear the window and bring it on top of the other windows
	XClearWindow(disp, window);
	XMapRaised(disp, window);
}

void window_main() {
	bool dirty = true;
	
	while (windowOpen) {
		if (dirty) {
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
	XSetForeground(disp, gc, style.text);
	CenterText(width / 2, 15, "Matrix Display");
	
	for (int x = 0; x < 32; x++) {
		word col = mem.mem_read(&cpu, &mem, 0xffc0 + x, true, mem.mem_ctx);
		for (int y = 0; y < 16; y++) {
			bool bit = (col >> y) & 1;
			XSetForeground(disp, gc, bit ? style.dispOff : style.dispOn);
			XFillRectangle(disp, window, gc, 10 + x*10, 20+y*10, 10, 10);
		}
	}
	
	
	
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
