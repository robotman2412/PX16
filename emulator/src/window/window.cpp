
#include <window.h>
#include <main.h>
#include <runner.h>



std::string escapeMarkup(std::string &other) {
	std::string out;
	
	for (char c: other) {
		if (c == '<') {
			out += "&lt;";
		} else if (c == '>') {
			out += "&gt;";
		} else if (c == '&') {
			out += "&amp;";
		} else {
			out += c;
		}
	}
	
	return out;
}

std::string format(const char *fmt, ...) {
	va_list vargs;
	
	// Calculate mem requirements.
	va_start(vargs, fmt);
	int len = vsnprintf(NULL, 0, fmt, vargs);
	va_end(vargs);
	
	// Allocate memory.
	char *mem = (char *) malloc(len+1);
	if (!mem) throw std::bad_alloc();
	
	// Format string.
	va_start(vargs, fmt);
	vsnprintf(mem, len+1, fmt, vargs);
	va_end(vargs);
	
	// Draw text.
	std::string res = std::string(mem);
	free(mem);
	
	return res;
}

std::string mkMonoStr(uint32_t color, std::string str) {
	str = escapeMarkup(str);
	return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%s</span>", color, str.c_str());
}

std::string mkMonoHex(uint32_t color, int digits, uint32_t value) {
	return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%0*x</span>", color, digits, value);
}

std::string mkMonoDec(uint32_t color, int digits, int value) {
	return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%0*d</span>", color, digits, value);
}

std::string mkMonoCount(uint32_t color, uint64_t value) {
	return mkMonoCount(color, "", value);
}

std::string mkMonoCount(uint32_t color, std::string unit, uint64_t value) {
	if (value >= 900000000000000) {
		return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%.1f P%s</span>", color, value / 1000000000000000.0, unit.c_str());
	} else if (value >= 900000000000) {
		return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%.1f T%s</span>", color, value / 1000000000000.0, unit.c_str());
	} else if (value >= 900000000) {
		return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%.1f G%s</span>", color, value / 1000000000.0, unit.c_str());
	} else if (value >= 900000) {
		return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%.1f M%s</span>", color, value / 1000000.0, unit.c_str());
	} else if (value >= 900) {
		return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%.1f K%s</span>", color, value / 1000.0, unit.c_str());
	} else {
		return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%llu  %s</span>", color, value, unit.c_str());
	}
}



Display::Display() {
	set_size_request(200, 100);
}

Display::~Display() {}

void Display::col2double(uint32_t in, double *out) {
	for (int i = 0; i < 3; i++) {
		out[i] = ((in >> (16 - i*8)) & 255) / 255.0;
	}
}

bool Display::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
	static bool size_adjusted = false;
	if (!size_adjusted) {
		int parent_width = get_parent()->get_width();
		set_size_request(-1, parent_width / 2);
		// size_adjusted = true;
	}
	
	// Get drawable width and height.
	Gtk::Allocation allocation = get_allocation();
	const int width  = allocation.get_width();
	const int height = allocation.get_height();
	
	// Scale drawing efforts.
	cr->save();
	cr->scale(width / 32.0, height / 16.0);
	
	double off_rgb[3];
	double on_rgb[3];
	col2double(style.dispOff, off_rgb);
	col2double(style.dispOn,  on_rgb);
	
	// Draw display pixels.
	for (int x = 0; x < 32; x++) {
		// Read VRAM.
		word value = mem.mem_read(&cpu, &mem, 0xffc0 + x, true, mem.mem_ctx);
		
		for (int y = 0; y < 16; y++) {
			// Determine bit value.
			bool bit = (value >> y) & 1;
			
			// Draw a RECTANGLE.
			cr->rectangle(x, y, 1, 1);
			if (bit) {
				cr->set_source_rgb(on_rgb[0],  on_rgb[1],  on_rgb[2]);
			} else {
				cr->set_source_rgb(off_rgb[0], off_rgb[1], off_rgb[2]);
			}
			cr->fill();
		}
	}
	cr->restore();
	
	return true;
}



MainWindow::MainWindow() {
	set_border_width(10);
	set_default_size(100, 100);
	set_title("Pixie 16");
	
	// Add the display stuff.
	displayLabel.set_markup("Matrix Display");
	cpuGrid.attach(displayLabel, 0, 0);
	cpuGrid.attach(display, 0, 1);
	
	// Add the registers stuff.
	regsLabel.set_markup("Registers");
	cpuGrid.attach(regsLabel, 0, 4);
	updateRegs();
	
	// Create registers grid.
	{
		// Register names.
		const char *regNames[2][7] = {{
			"R0",   "R1",   "R2",  "R3",  "ST", "PF", "PC",
		}, {
			"Imm0", "Imm1", "PbA", "PbB", "AR", "Db", "Ab",
		}};
		
		regsGrid.set_column_spacing(10);
		regsGrid.set_row_spacing(5);
		for (int y = 0; y < 2; y++) {
			for (int x = 0; x < 7; x++) {
				// I wish there were a better way to do this...
				uint32_t col = y ? style.regsHidden : x > 3 ? style.regsSpecial : style.regsGeneral;
				regsNames[x][y].set_markup(mkMonoStr(col, regNames[y][x]));
				regsNames[x][y].set_alignment(Gtk::ALIGN_START);
				
				// Add register names and values to grid.
				regsGrid.attach(regsNames[x][y],  x, y*2);
				regsGrid.attach(regsValues[x][y], x, y*2+1);
				
				regsNames[x][y].set_hexpand(true);
				regsNames[x][y].set_halign(Gtk::ALIGN_FILL);
				regsValues[x][y].set_hexpand(true);
				regsValues[x][y].set_halign(Gtk::ALIGN_FILL);
			}
		}
		cpuGrid.attach(regsGrid, 0, 5);
	}
	
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
			sim_total_ticks = 0;
		});
		ctlGrid.attach(resetButton, 0, 3);
	}
	
	// Open debugger button.
	{
		openDebuggerButton = Gtk::Button("Open debugger");
		openDebuggerButton.signal_clicked().connect([this]() -> void {
			Debugger *debugger = new Debugger();
			debugger->parent = this;
			debuggers.push_back(debugger);
			debugger->show();
		});
		ctlGrid.attach(openDebuggerButton, 0, 5);
	}
	
	ctlGrid.set_hexpand(false);
	ctlGrid.set_vexpand(false);
	
	cpuGrid.set_hexpand(true);
	cpuGrid.set_vexpand(true);
	cpuGrid.set_halign(Gtk::ALIGN_FILL);
	cpuGrid.set_valign(Gtk::ALIGN_FILL);
	
	// Show everything.
	mainContainer.attach(cpuGrid, 0, 0);
	mainContainer.attach(ctlGrid, 1, 0);
	ctlGrid.set_margin_left(10);
	add(mainContainer);
	show_all();
	
	// Set main timer.
	mainTimer = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::update), 16);
}

MainWindow::~MainWindow() {}

// Update button styles.
void MainWindow::updateButtons() {
	static bool lastRunning = false;
	static bool lastWarp    = false;
	
	if (lastRunning == running && lastWarp == warp_speed) return;
	
	// Run/stop button.
	runStopButton.set_label(running ? "Stop" : "Run");
	runStopButton.set_image_from_icon_name(running ? "media-playback-stop" : "media-playback-start");
	
	// Warp speed button.
	warpButton.set_label(warp_speed ? "End warp" : "Warp speed");
	warpButton.set_image_from_icon_name(warp_speed ? "media-skip-forward" : "media-seek-forward");
	
	lastRunning = running;
	lastWarp    = warp_speed;
}

bool MainWindow::update() {
	updateButtons();
	updateRegs();
	display.queue_draw();
	return true;
}

void MainWindow::updateRegs() {
	static word prevRegfile[7];
	static word prevHregs[7];
	static bool init = false;
	char tmp[5];
	
	// Get hidden register values.
	word hregs[7] = {
		cpu.imm0, cpu.imm1, cpu.par_bus_a, cpu.par_bus_b, cpu.AR, cpu.data_bus, cpu.addr_bus
	};
	
	// If not the first run, and equals last run, don't do anything.
	if (init && !memcmp(hregs, prevHregs, sizeof(prevHregs)) && !memcmp(prevRegfile, cpu.regfile, sizeof(prevRegfile))) return;
	
	// Format "normal registers"
	for (int x = 0; x < 7; x++) {
		regsValues[x][0].set_markup(mkMonoHex(style.regsValue, 4, cpu.regfile[x]));
	}
	
	// Format "hidden registers"
	for (int x = 0; x < 7; x++) {
		regsValues[x][1].set_markup(mkMonoHex(style.regsValue, 4, hregs[x]));
	}
	
	// Copy back last run info.
	memcpy(prevRegfile, cpu.regfile, sizeof(prevRegfile));
	memcpy(prevHregs,   hregs,       sizeof(prevHregs));
	init = true;
}



style_t style = DEFAULT_STYLE();

void window_main() {
	auto app = Gtk::Application::create();
	MainWindow mainWindow;
	app->run(mainWindow);
}
