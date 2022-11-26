
#include <window.h>
#include <main.h>

#include <cairomm/context.h>

#include <cmath>

#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>

#include <pthread.h>



static std::string format(const char *fmt, ...) {
	va_list vargs;
	
	// Calculate mem requirements.
	va_start(vargs, fmt);
	int len = vsnprintf(NULL, 0, fmt, vargs);
	va_end(vargs);
	
	// Allocate memory.
	char *mem = (char *) malloc(len+1);
	if (!mem) return std::string("(null)");
	
	// Format string.
	va_start(vargs, fmt);
	vsnprintf(mem, len+1, fmt, vargs);
	va_end(vargs);
	
	// Draw text.
	std::string res = std::string(mem);
	free(mem);
	
	return res;
}

static std::string mkMonoStr(uint32_t color, const char *str) {
	return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%s</span>", color, str);
}

static std::string mkMonoHex(uint32_t color, int digits, uint32_t value) {
	return format("<span font-weight=\"bold\" font-family=\"FreeMono\" color=\"#%06x\">%0*x</span>", color, digits, value);
}



class Display : public Gtk::DrawingArea {
	public:
		Display();
		virtual ~Display();
		
		static void col2double(uint32_t in, double *out);
		virtual bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context>& cr);
};

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
		set_size_request(parent_width, parent_width / 2);
		size_adjusted = true;
	}
	
	// OBTAIN DIMENSIONS.
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
			bool bit = (x ^ y) & 1; //(value >> y) & 1;
			
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



class MainWindow : public Gtk::Window {
	public:
		MainWindow();
		virtual ~MainWindow();
		
		bool update();
		void updateRegs();
		
		// The timer used to continually update stuff.
		sigc::connection mainTimer;
		// The main layout containing grid.
		Gtk::Grid mainGrid;
		
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
};

MainWindow::MainWindow() {
	set_border_width(10);
	set_default_size(100, 100);
	set_title("Pixie 16");
	
	// Add the display stuff.
	displayLabel.set_markup("Matrix Display");
	mainGrid.attach(displayLabel, 0, 0);
	mainGrid.attach(display, 0, 1);
	
	// Add the registers stuff.
	regsLabel.set_markup("Registers");
	mainGrid.attach(regsLabel, 0, 4);
	updateRegs();
	
	// Register names.
	const char *regNames[2][7] = {{
		"R0  ", "R1  ", "R2  ", "R3  ", "ST  ", "PF  ", "PC  ",
	}, {
		"Imm0", "Imm1", "PbA ", "PbB ", "AR  ", "Db  ", "Ab  ",
	}};
	
	// Create registers grid.
	regsGrid.set_column_spacing(10);
	regsGrid.set_row_spacing(5);
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 7; x++) {
			// I wish there were a better way to do this...
			uint32_t col = y ? style.regsHidden : x > 3 ? style.regsSpecial : style.regsGeneral;
			regsNames[x][y].set_markup(mkMonoStr(col, regNames[y][x]));
			
			// Add register names and values to grid.
			regsGrid.attach(regsNames[x][y],  x, y*2);
			regsGrid.attach(regsValues[x][y], x, y*2+1);
		}
	}
	mainGrid.attach(regsGrid, 0, 5);
	
	// Show everything.
	add(mainGrid);
	mainGrid.show_all();
	
	// Set main timer.
	mainTimer = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::update), 16);
}

MainWindow::~MainWindow() {}

bool MainWindow::update() {
	updateRegs();
	return true;
}

void MainWindow::updateRegs() {
	char tmp[5];
	
	// Format "normal registers"
	for (int x = 0; x < 7; x++) {
		regsValues[x][0].set_markup(mkMonoHex(style.regsValue, 4, cpu.regfile[x]));
	}
	
	// Get hidden register values.
	word hregs[7] = {
		cpu.imm0, cpu.imm1, cpu.par_bus_a, cpu.par_bus_b, cpu.AR, cpu.data_bus, cpu.addr_bus
	};
	
	// Format "hidden registers"
	for (int x = 0; x < 7; x++) {
		regsValues[x][1].set_markup(mkMonoHex(style.regsValue, 4, hregs[x]));
	}
}



style_t style = DEFAULT_STYLE();



void window_main() {
	auto app = Gtk::Application::create();
	MainWindow mainWindow;
	app->run(mainWindow);
}
