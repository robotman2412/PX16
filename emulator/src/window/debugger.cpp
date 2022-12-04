
#include "debugger.h"
#include <main.h>
#include <window.h>
#include <runner.h>

Debugger::Debugger() {
	set_border_width(10);
	set_default_size(500, 750);
	set_size_request(400, 300);
	set_title("Pixie debugger");
	
	// Controls label.
	controlsLabel = Gtk::Label("Controls");
	ctlGrid.attach(controlsLabel, 0, 0);
	
	// Run/Stop button.
	{
		runStopButton = Gtk::Button("Run");
		runStopButton.set_image_from_icon_name("media-playback-start");
		runStopButton.set_always_show_image(true);
		runStopButton.signal_clicked().connect([this]() -> void {
			running = !running;
			if (!running) {
				warp_speed = false;
			}
		});
		runStopButton.set_size_request(120, -1);
		ctlGrid.attach(runStopButton, 0, 1);
	}
	
	// Warp speed button.
	{
		warpButton = Gtk::Button("Warp speed");
		warpButton.set_image_from_icon_name("media-seek-forward");
		warpButton.set_always_show_image(true);
		warpButton.signal_clicked().connect([this]() -> void {
			running = true;
			warp_speed = !warp_speed;
		});
		ctlGrid.attach(warpButton, 0, 2);
	}
	
	// Reset button.
	{
		resetButton = Gtk::Button("Reset");
		resetButton.set_image_from_icon_name("view-refresh");
		resetButton.set_always_show_image(true);
		resetButton.signal_clicked().connect([this]() -> void {
			running    = false;
			warp_speed = false;
			runner_join();
			core_reset(&cpu, true);
			if (mem.reset) mem.reset(&cpu, &mem, mem.mem_ctx, true);
		});
		ctlGrid.attach(resetButton, 0, 3);
	}
	
	// Instruction step button.
	{
		insnStepButton = Gtk::Button("Step instruction");
		insnStepButton.signal_clicked().connect([this]() -> void {
			if (!running) sim_total_ticks += fast_tick(&cpu, &mem);
		});
		ctlGrid.attach(insnStepButton, 0, 4);
	}
	
	// Step over button.
	{
		stepOverButton = Gtk::Button("Step over");
		stepOutButton.signal_clicked().connect([this]() -> void {
			if (!running) {
				sim_mode = TICK_STEP_OVER;
				running  = true;
			}
		});
	}
	
	// Add source label.
	sourceLabel.set_markup("Code view");
	rightGrid.attach(sourceLabel, 0, 0);
	
	// Add source code container.
	sourceScroller.add(sourceContainer);
	sourceScroller.set_hexpand(true);
	sourceScroller.set_vexpand(true);
	sourceScroller.set_halign(Gtk::ALIGN_FILL);
	sourceScroller.set_valign(Gtk::ALIGN_FILL);
	rightGrid.attach(sourceScroller, 0, 1);
	
	// Add right grid.
	rightGrid.set_hexpand(true);
	rightGrid.set_vexpand(true);
	rightGrid.set_halign(Gtk::ALIGN_FILL);
	rightGrid.set_valign(Gtk::ALIGN_FILL);
	mainContainer.attach(rightGrid, 1, 0);
	
	// Add controls grid.
	mainContainer.attach(ctlGrid, 0, 0);
	
	// Add main container.
	mainContainer.set_hexpand(true);
	mainContainer.set_vexpand(true);
	mainContainer.set_halign(Gtk::ALIGN_FILL);
	mainContainer.set_valign(Gtk::ALIGN_FILL);
	mainContainer.set_column_spacing(10);
	add(mainContainer);
	
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
	currHighlight = -1;
	
	// Resize arrays.
	size_t lineCount = map.lineList.size();
	sourceAddrs.resize(lineCount);
	sourceLines.resize(lineCount);
	sourceCode.resize(lineCount);
	
	// Add some stuff.
	for (size_t i = 0; i < lineCount; i++) {
		SourceLine &line = map.lineList[i];
		
		// Instantiate elements.
		if (line.addresses.size()) {
			sourceAddrs[i].set_markup(mkMonoHex(style.text, 4, line.addresses[0]));
		} else {
			sourceAddrs[i].set_markup("");
		}
		sourceLines[i].set_markup(mkMonoDec(style.text, 1, line.linenumber));
		sourceCode [i].set_markup(line.formatted);
		
		// Set alignments.
		sourceAddrs[i].set_alignment(Gtk::ALIGN_START);
		sourceLines[i].set_alignment(Gtk::ALIGN_END);
		sourceCode [i].set_alignment(Gtk::ALIGN_START);
		
		// Add elements to the grid.
		sourceContainer.attach(sourceAddrs[i], 0, i);
		sourceContainer.attach(sourceLines[i], 1, i);
		sourceContainer.attach(sourceCode [i], 2, i);
		
		// Map line in addr2line mapppppppp.
		for (word addr : line.addresses) {
			addr2line[addr] = i;
		}
	}
	
	// Add the things back.
	sourceScroller.add(sourceContainer);
	show_all();
	
	// Update VISUALS.
	update();
}

// Update button styles.
void Debugger::updateButtons() {
	static bool lastRunning = false;
	static bool lastWarp    = false;
	
	if (lastRunning == running && lastWarp == warp_speed) return;
	
	// Run/stop button.
	runStopButton.set_label(running ? "Stop" : "Run");
	runStopButton.set_image_from_icon_name(running ? "media-playback-stop" : "media-playback-start");
	
	// Warp speed button.
	warpButton.set_label(warp_speed ? "End warp" : "Warp speed");
	warpButton.set_image_from_icon_name(warp_speed ? "media-skip-forward" : "media-seek-forward");
	
	// Instruction step button.
	insnStepButton.set_sensitive(!running);
	
	lastRunning = running;
	lastWarp    = warp_speed;
}

// Update the addr2line device.
bool Debugger::update() {
	// The target line to highlight.
	ssize_t newHighlight = -1;
	
	// Look for PC in the addr2line map.
	word PC   = (cpu.state.boot_0 || cpu.state.boot_1) ? mem.mem_read(&cpu, &mem, 2, true, mem.mem_ctx) : cpu.PC;
	auto iter = addr2line.find(PC);
	if (iter != addr2line.end()) {
		newHighlight = iter->second;
	}
	
	// Clear the current highlight if required.
	if (currHighlight >= 0 && newHighlight != currHighlight) {
		sourceCode[currHighlight].unset_background_color();
	}
	
	// Set the new highlight if required.
	if (newHighlight >= 0 && newHighlight != currHighlight) {
		// Make a color.
		uint32_t raw = style.debugPC;
		Gdk::RGBA col;
		col.set_alpha_u(0xffff);
		col.set_red_u  ((uint8_t) (raw >> 16) * 0x101);
		col.set_green_u((uint8_t) (raw >>  8) * 0x101);
		col.set_blue_u ((uint8_t)  raw        * 0x101);
		
		// Apply colors.
		sourceCode[newHighlight].override_background_color(col);
		
		// Scroll to SOURCE LINE.
		auto   ptr    = sourceScroller.get_vadjustment();
		double scroll = sourceCode[newHighlight].get_allocation().get_y() - sourceScroller.get_allocated_height() / 3.0;
		if (scroll < ptr->get_lower()) scroll = ptr->get_lower();
		if (scroll > ptr->get_upper()) scroll = ptr->get_upper();
		ptr->set_value(scroll);
	}
	
	// Update button styles.
	updateButtons();
	
	currHighlight = newHighlight;
	
	return true;
}
