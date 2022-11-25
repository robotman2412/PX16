
#include "debugger.h"
#include <px16.h>
#include <main.h>
#include <math.h>

static int       screen;
static Window    win;
static GC        gc;

static int mouseX =   0, mouseY =   0;
static int width  = 340, height = 400;

static int memX, memY, memW, memH;
static int codeX, codeY, codeW, codeH;

bool debuggerOpen = false;
static const char *fontName = "8x13bold";
static XFontStruct *font;

static bool dirty = true;

static Atom deleteWindowAtom;

static word addrOffs   = 0;
static int  addrCursor = 0x0034;

static void calcLayout() {
	codeX = 10;
	codeY = 0;
	codeW = width / 2 - 15;
	codeH = height - 10;
	
	memX = width / 2 + 5;
	memY = 0;
	memW = width / 2 - 15;
	memH = height - 10;
}

static void enterHex(word hv) {
	if (addrCursor < 0 || addrCursor > 0xffff) return;
	
	word orig = mem.mem_read(&cpu, &mem, addrCursor, true, mem.mem_ctx);
	word next = (orig << 4) | hv;
	mem.mem_write(&cpu, &mem, addrCursor, next, mem.mem_ctx);
	
	dirty = true;
}

static void selectMemory(int dx, int dy) {
	if (addrCursor < 0 || addrCursor > 0xffff) return;
	
	int cols  = (1 << (int) log2l(memW / 50));
	int rows  = (memH - 20) / 15;
	
	addrCursor += dx + dy * cols;
	
	if (addrCursor < 0) addrCursor = 0;
	if (addrCursor > 0xffff) addrCursor = 0xffff;
	
	dirty = true;
}



static void drawMemory() {
	int cols  = (1 << (int) log2l(memW / 50));
	int rows  = (memH - 20) / 15;
	int cellW = memW / (cols + 1);
	int cellH = 15;
	
	XSetForeground(disp, gc, style.text);
	CenterText(disp, win, gc, memX + memW / 2, memY + 15, "Memory");
	
	// Draw memory grid.
	for (int row = 0; row < rows; row++) {
		// Draw address.
		XSetForeground(disp, gc, style.memoryRAM);
		XFillRectangle(disp, win, gc, memX, memY + 20 + row * cellH, cellW, cellH);
		
		XSetForeground(disp, gc, style.memoryAddr);
		CenterTextf(disp, win, gc, memX + cellW / 2, memY + 33 + row * cellH, "%04x", addrOffs + cols * row);
		
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
			XFillRectangle(disp, win, gc, memX + (col + 1) * cellW, memY + 20 + row * cellH, cellW, cellH);
			
			// Draw value.
			XSetForeground(disp, gc, style.memoryText);
			CenterTextf(disp, win, gc, memX + cellW / 2 + (col + 1) * cellW, memY + 33 + row * cellH, "%04x", value);
		}
	}
	
	// Draw memory borders.
	XSetForeground(disp, gc, 0);
	XDrawRectangle(disp, win, gc, memX, memY + 20, cellW * (cols + 1), cellH * rows);
	XDrawLine(disp, win, gc, memX + cellW, memY + 20, memX + cellW, memY + 20 + cellH * rows);
	
	// Is selected address on screen?
	if (addrCursor >= addrOffs && addrCursor < addrOffs + rows * cols) {
		int cursorX = (addrCursor - addrOffs) % cols;
		int cursorY = (addrCursor - addrOffs) / cols;
		
		XSetForeground(disp, gc, style.memorySel);
		XDrawRectangle(disp, win, gc, memX + (cursorX + 1) * cellW, memY + 20 + cursorY * cellH, cellW, cellH);
	}
	
}

static void drawCode() {
	
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
	XSelectInput(
		disp, win,
		ExposureMask
		| ButtonPressMask | ButtonReleaseMask
		| KeyPressMask | KeyReleaseMask
		| StructureNotifyMask | PointerMotionMask
	);
	
	// create the Graphics Context
	gc = XCreateGC(disp, win, 0, 0);
	
	font = XLoadQueryFont(disp, fontName);
	if (font) XSetFont(disp, gc, font->fid);
	
	// clear the win and bring it on top of the other windows
	XClearWindow(disp, win);
	calcLayout();
	
	// Add window closing atom.
	deleteWindowAtom = XInternAtom(disp, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(disp, win, &deleteWindowAtom, 1);
	
	debuggerOpen = true;
}

void debugger_redraw() {
	drawMemory();
}

void debugger_loop() {
	if (dirty && debuggerOpen) {
		debugger_redraw();
		dirty = false;
	}
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
		calcLayout();
		dirty = true;
	} else if (msg.type == KeyPress && msg.xkey.window == win) {
		// int keyChar = XKeycodeToKeysym(disp, msg.xkey.keycode, msg.xkey.state);
		int keyChar = XkbKeycodeToKeysym(disp, msg.xkey.keycode, 0, msg.xkey.state & ShiftMask ? 1 : 0);
		
		if (keyChar >= '0' && keyChar <= '9') {
			enterHex(keyChar - '0');
		} else if (keyChar >= 'a' && keyChar <= 'f') {
			enterHex(keyChar - 'a' + 0xa);
		} else if (keyChar >= 'A' && keyChar <= 'F') {
			enterHex(keyChar - 'A' + 0xa);
		} else if (keyChar == XK_Up) {
			selectMemory(0, -1);
		} else if (keyChar == XK_Down) {
			selectMemory(0, 1);
		} else if (keyChar == XK_Left) {
			selectMemory(-1, 0);
		} else if (keyChar == XK_Right) {
			selectMemory(1, 0);
		}
	}
	
	// handleButtonEvent(&runButton,  msg);
	// handleButtonEvent(&stepButton, msg);
	// handleButtonEvent(&warpButton, msg);
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

// Loads source code for the running binary from addr2line dump.
void debugger_load(FILE *fd) {
	
}
