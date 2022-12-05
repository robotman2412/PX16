
#ifndef WINDOW_H
#define WINDOW_H

#include <cmath>
#include <gtkmm.h>
#include <pthread.h>
#include <stdexcept>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <debugger.h>



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
	
	// Debugger current line background.
	uint32_t debugPC;
} style_t;

extern const char *style_names[];
extern const size_t n_style_names;

#define DEFAULT_STYLE() (style_t) { \
	.background  = 0x2f2f2f, \
	.dispOff     = 0x3f3f3f, \
	.dispOn      = 0x4f7fff, \
	.text        = 0xefefef, \
	.regsValue   = 0x4f7fff, \
	.regsGeneral = 0xefdf9f, \
	.regsSpecial = 0xdf5fff, \
	.regsHidden  = 0xcf3f3f, \
	.memoryRAM   = 0x3f3f3f, \
	.memoryROM   = 0x5f2f2f, \
	.memoryMMIO  = 0x5f5f2f, \
	.memoryVRAM  = 0x5f5f2f, \
	.memorySel   = 0xcfcfff, \
	.memoryText  = 0xcfcfcf, \
	.memoryAddr  = 0xafafaf, \
	.debugPC     = 0x6f2f2f, \
}



class Display: public Gtk::DrawingArea {
	public:
		Display();
		virtual ~Display();
		
		static void col2double(uint32_t in, double *out);
		virtual bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context>& cr);
};

class MainWindow: public Gtk::Window {
	public:
		MainWindow();
		virtual ~MainWindow();
		
		bool update();
		void updateRegs();
		// Update button styles.
		void updateButtons();
		
		// Active debugger windows.
		std::vector<Debugger*> debuggers;
		
		// The timer used to continually update registers, display, etc.
		sigc::connection mainTimer;
		
		// The main container for all the things.
		Gtk::Grid mainContainer;
		// The grid that contains the CPU info, registers, display, etc.
		Gtk::Grid cpuGrid;
		// The grid that contains the controls, statistics, etc.
		Gtk::Grid ctlGrid;
		
		// Label: Matrix Display
		Gtk::Label displayLabel;
		// Matrix displaying device.
		Display    display;
		
		// Label: Statistics.
		
		// Label: Registers.
		Gtk::Label regsLabel;
		// Registers value container.
		Gtk::Grid  regsGrid;
		// Register names.
		Gtk::Label regsNames[7][2];
		// Register values.
		Gtk::Label regsValues[7][2];
		
		// Label: Controls.
		Gtk::Label controlsLabel;
		
		// Button: Run/stop.
		Gtk::Button runStopButton;
		// Button: Fast forward.
		Gtk::Button warpButton;
		// Button: Reset.
		Gtk::Button resetButton;
		// Button: Open debugger.
		Gtk::Button openDebuggerButton;
};



std::string escapeMarkup(std::string &other);
std::string format(const char *fmt, ...);
std::string mkMonoStr(uint32_t color, std::string str);
std::string mkMonoHex(uint32_t color, int digits, uint32_t value);
std::string mkMonoDec(uint32_t color, int digits, int value);
std::string mkMonoCount(uint32_t color, uint64_t value);
std::string mkMonoCount(uint32_t color, std::string suffix, uint64_t value);



extern style_t style;

void window_main();

#endif // WINDOW_H
