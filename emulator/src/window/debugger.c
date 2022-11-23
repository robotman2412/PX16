
#include "debugger.h"
#include <px16.h>
#include <main.h>
#include <math.h>

static int       screen;
static Window    win;
static GC        gc;

static int mouseX =   0, mouseY =   0;
static int width  = 340, height = 400;

bool debuggerOpen = false;
static const char *fontName = "8x13bold";
static XFontStruct *font;

static bool dirty = true;

static Atom deleteWindowAtom;

static void drawMemory(int x, int y, int width, int height) {
	XSetForeground(disp, gc, style.text);
	CenterText(disp, win, gc, x + width / 2, y + 15, "Memory");
	
	int cols = (1 << (int) log2l(width / 50));
	int rows = (height - 20) / 15;
	int cellWidth  = width / (cols + 1);
	int cellHeight = 15;
	
	// Draw memory grid.
	word addrOffs   = 0;
	word addrCursor = 0;
	for (int row = 0; row < rows; row++) {
		// Draw address.
		XSetForeground(disp, gc, style.memoryRAM);
		XFillRectangle(disp, win, gc, x, y + 20 + row * cellHeight, cellWidth, cellHeight);
		
		XSetForeground(disp, gc, style.memoryAddr);
		CenterTextf(disp, win, gc, x + cellWidth / 2, y + 33 + row * cellHeight, "%04x", addrOffs + cols * row);
		
		// Draw cells in the row.
		for (int col = 0; col < cols; col++) {
			// Read memory associated.
			word addr = addrOffs + cols * row + col;
			word value = mem.mem_read(&cpu, &mem, addr, true, mem.mem_ctx);
			memtype type = mem.mem_gettype(&cpu, &mem, addr, mem.mem_ctx);
			
			// Draw background.
			if (type == MEM_TYPE_RAM)  XSetForeground(disp, gc, style.memoryRAM);
			if (type == MEM_TYPE_ROM)  XSetForeground(disp, gc, style.memoryROM);
			if (type == MEM_TYPE_MMIO) XSetForeground(disp, gc, style.memoryMMIO);
			if (type == MEM_TYPE_VRAM) XSetForeground(disp, gc, style.memoryVRAM);
			XFillRectangle(disp, win, gc, x + (col + 1) * cellWidth, y + 20 + row * cellHeight, cellWidth, cellHeight);
			
			// Draw value.
			XSetForeground(disp, gc, style.memoryText);
			CenterTextf(disp, win, gc, x + cellWidth / 2 + (col + 1) * cellWidth, y + 33 + row * cellHeight, "%04x", value);
		}
	}
	
	// Draw memory borders.
	XSetForeground(disp, gc, 0);
	XDrawRectangle(disp, win, gc, x, y + 20, cellWidth * (cols + 1), cellHeight * rows);
	XDrawLine(disp, win, gc, x + cellWidth, y + 20, x + cellWidth, y + 20 + cellHeight * rows);
	
	// Is selected address on screen?
	if (addrCursor >= addrOffs && addrCursor < addrOffs + rows * cols) {
		int cursorX = (addrCursor - addrOffs) % rows;
		int cursorY = (addrCursor - addrOffs) / rows;
		
		XSetForeground(disp, gc, style.memorySel);
		XDrawRectangle(disp, win, gc, x + (cursorX + 1) * cellWidth, y + 20 + cursorY * cellHeight, cellWidth, cellHeight);
	}
	
}



void debugger_init() {
	// use the information from the environment variable DISPLAY to create the X connection:
	// disp   = XOpenDisplay(NULL);
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
	
	// Add window closing atom.
	deleteWindowAtom = XInternAtom(disp, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(disp, win, &deleteWindowAtom, 1);
	
	debuggerOpen = true;
}

void debugger_redraw() {
	drawMemory(10, 0, width - 20, height - 10);
}

void debugger_event(XEvent msg) {
	if (msg.type == ClientMessage) {
		if ((Atom) msg.xclient.data.l[0] == deleteWindowAtom && msg.xclient.window == win) {
			debugger_hide();
			return;
		}
	} else if (msg.type == MotionNotify && msg.xmotion.window == win) {
		mouseX = msg.xmotion.x;
		mouseY = msg.xmotion.y;
		dirty = true;
	} else if (msg.type == ConfigureNotify && msg.xconfigure.window == win) {
		width  = msg.xconfigure.width;
		height = msg.xconfigure.height;
		dirty = true;
	}
	
	// handleButtonEvent(&runButton,  msg);
	// handleButtonEvent(&stepButton, msg);
	// handleButtonEvent(&warpButton, msg);
	
	if (dirty) {
		debugger_redraw();
		dirty = false;
	}
}

void debugger_show() {
	XMapRaised(disp, win);
}

void debugger_hide() {
	XUnmapWindow(disp, win);
}

void debugger_close() {
	if (debuggerOpen) {
		XUnmapWindow(disp, win);
		XDestroyWindow(disp, win);
		XFreeGC(disp, gc);
	}
	debuggerOpen = false;
}
