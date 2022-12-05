
#pragma once

#include <cmath>
#include <gtkmm.h>
#include <pthread.h>
#include <stdexcept>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <progmap.h>

class Debugger: public Gtk::Window {
	public:
		Debugger();
		virtual ~Debugger();
		
		// Select new program map.
		void setMap(ProgMap newMap);
		
		// Update the code view.
		bool update();
		// Update button styles.
		void updateButtons();
		// Update statistics.
		void updateStats();
		
		// Currently highlighted line in code view.
		ssize_t             currHighlight;
		// The program map on which the code view is based.
		ProgMap             map;
		
		// The timer used to continually update registers, display, etc.
		sigc::connection    mainTimer;
		
		/* ==== STATIC ELEMENTS ==== */
		
		// The main container for all the things.
		Gtk::Grid   mainContainer;
		
		// The grid on the left.
		Gtk::Grid   leftGrid;
		// The grid on the right.
		Gtk::Grid   rightGrid;
		
		// The grid that contains controls.
		Gtk::Grid   ctlGrid;
		// The label for statistics.
		Gtk::Label  statsLabel;
		// The grid that contains statistics.
		Gtk::Grid   statsGrid;
		
		// The label for source code.
		Gtk::Label  sourceLabel;
		// The scrolling bar device for code view.
		Gtk::ScrolledWindow sourceScroller;
		// The container for code view.
		Gtk::Grid   sourceContainer;
		
		// The label for controls.
		Gtk::Label  controlsLabel;
		// Button: Run/stop.
		Gtk::Button runStopButton;
		// Button: Fast forward.
		Gtk::Button warpButton;
		// Button: Reset.
		Gtk::Button resetButton;
		// Button: Instruction step.
		Gtk::Button insnStepButton;
		// Button: Line step / step in.
		Gtk::Button lineStepButton;
		// Button: Step over function.
		Gtk::Button stepOverButton;
		// Button: Step out button.
		Gtk::Button stepOutButton;
		
		// The name for speed.
		Gtk::Label  speedName;
		// The value for speed.
		Gtk::Label  speedValue;
		// The name for cycle count.
		Gtk::Label  cyclesName;
		// The value for cycle count.
		Gtk::Label  cyclesValue;
		// The name for instruction count.
		Gtk::Label  insnName;
		// The value for instruction count.
		Gtk::Label  insnValue;
		// The name for JSR count.
		Gtk::Label  jsrName;
		// The value for JSR count.
		Gtk::Label  jsrValue;
		
		// The label for variables.
		Gtk::Label          varLabel;
		// The scrolling bar device for variables.
		Gtk::ScrolledWindow varScroller;
		// The container for variables.
		Gtk::Grid           varContainer;
		
		/* ==== DYNAMIC ELEMENTS ==== */
		
		// Addresses for code view.
		std::vector<Gtk::Label> sourceAddrs;
		// Linenumbers for code view.
		std::vector<Gtk::Label> sourceLines;
		// File contents for code view.
		std::vector<Gtk::Label> sourceCode;
		// Address to line map for code view.
		std::map<word, size_t>  addr2line;
		
		// Names vor variables.
		std::vector<Gtk::Label> varNames;
		// Values for variables.
		std::vector<Gtk::Label> varValues;
};
