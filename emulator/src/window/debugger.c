
#include "debugger.h"

static Display  *disp;
static int       screen;
static Window    win;
static GC        gc;

static int mouseX =   0, mouseY =   0;
static int width  = 340, height = 400;

bool debuggerOpen = false;
static const char *fontName = "8x13bold";
static XFontStruct *font;

void debugger_init() {
	// use the information from the environment variable DISPLAY to create the X connection:
	disp   = XOpenDisplay(NULL);
	screen = DefaultScreen(disp);
	
	// Once the display is initialized, create the win.
	// This win will be have be 200 pixels across and 300 down.
	// It will have the foreground white and background black.
	win = XCreateSimpleWindow(disp, DefaultRootWindow(disp), 0, 0, width, height, 5, 0xffffff, style.background);
	
	// here is where some properties of the win can be set.
	// The third and fourth items indicate the name which appears at the top of the win and the name of the minimized win respectively.
	XSetStandardProperties(disp, win, "Pixie Debugger", "pxdb.png", None, NULL, 0, NULL);
	
	// this routine determines which types of input are allowed in the input.
	// see the appropriate section for details...
	XSelectInput(disp, win, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask);
	
	// create the Graphics Context
	gc = XCreateGC(disp, win, 0, 0);
	
	font = XLoadQueryFont(disp, fontName);
	if (font) XSetFont(disp, gc, font->fid);
	
	// clear the win and bring it on top of the other windows
	XClearWindow(disp, win);
	XMapRaised(disp, win);
	
	debuggerOpen = true;
}

bool debugger_poll() {
	XEvent msg;
	
	if (XCheckWindowEvent(disp, win, -1, &msg)) {
		if (msg.type == DestroyNotify) {
			debuggerOpen = false;
			printf("Closed i think\n");
			return true;
		} else if (msg.type == MotionNotify) {
			mouseX = msg.xmotion.x;
			mouseY = msg.xmotion.y;
		} else if (msg.type == ConfigureNotify) {
			width  = msg.xconfigure.width;
			height = msg.xconfigure.height;
		}
		
		// handleButtonEvent(&runButton,  msg);
		// handleButtonEvent(&stepButton, msg);
		// handleButtonEvent(&warpButton, msg);
		
		// Handlage.
		return true;
	}
	return false;
}
