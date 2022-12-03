
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
		
		// Currently highlighted line in code view.
		ssize_t             highlightedIndex;
		// The program map on which the code view is based.
		ProgMap             map;
		
		// The timer used to continually update registers, display, etc.
		sigc::connection    mainTimer;
		
		// The main container for all the things.
		Gtk::Box            mainContainer;
		// The scrolling bar device for code view.
		Gtk::ScrolledWindow sourceScroller;
		// The container for code view.
		Gtk::Grid           sourceContainer;
		// Addresses for code view.
		std::vector<Gtk::Label> sourceAddrs;
		// Linenumbers for code view.
		std::vector<Gtk::Label> sourceLines;
		// File contents for code view.
		std::vector<Gtk::Label> sourceCode;
		// Address to line map for code view.
		std::map<word, size_t>  addr2line;
};
