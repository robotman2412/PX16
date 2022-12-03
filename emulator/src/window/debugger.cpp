
#include "debugger.h"
#include <main.h>
#include <window.h>

Debugger::Debugger() {
	set_border_width(10);
	set_default_size(500, 750);
	set_title("Pixie debugger");
	
	// Add source code device.
	sourceScroller.add(sourceContainer);
	// mainContainer.add(sourceScroller);
	add(sourceScroller);
	
	// add(mainContainer);
	show_all();
	
	setMap(ProgMap("/home/julian/the_projects/px16_bad_apple/build/exec.map"));
	
	// Set main timer.
	mainTimer = Glib::signal_timeout().connect(sigc::mem_fun(*this, &Debugger::update), 16);
}

Debugger::~Debugger() {}



// Select new program map.
void Debugger::setMap(ProgMap newMap) {
	// Delete old data.
	sourceScroller.remove();
	sourceContainer = Gtk::Grid();
	sourceContainer.set_column_spacing(10);
	map = newMap;
	highlightedIndex = -1;
	
	// Resize arrays.
	size_t size = map.lineList.size();
	sourceAddrs.resize(size);
	sourceLines.resize(size);
	sourceCode.resize(size);
	
	// Add some stuff.
	for (size_t i = 0; i < size; i++) {
		SourceLine &line = map.lineList[i];
		
		// Instantiate elements.
		sourceAddrs[i].set_markup(line.addressKnown ? mkMonoHex(style.text, 4, line.address) : "");
		sourceLines[i].set_markup(mkMonoDec(style.text, 1, line.linenumber));
		sourceCode [i].set_markup(line.formatted);
		
		// Set all alignment to left.
		sourceAddrs[i].set_alignment(Gtk::ALIGN_START);
		sourceLines[i].set_alignment(Gtk::ALIGN_START);
		sourceCode [i].set_alignment(Gtk::ALIGN_START);
		
		// Add elements to the grid.
		sourceContainer.attach(sourceAddrs[i], 0, i);
		sourceContainer.attach(sourceLines[i], 1, i);
		sourceContainer.attach(sourceCode [i], 2, i);
	}
	
	// Add the things back.
	sourceScroller.add(sourceContainer);
	show_all();
	
	// Update VISUALS.
	update();
}

// Update the addr2line device.
bool Debugger::update() {
	return true;
}
