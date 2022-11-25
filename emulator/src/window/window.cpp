
#include <window.h>

#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/grid.h>



class TestWin : public Gtk::Window {
	public:
		TestWin();
		virtual ~TestWin();
		
	protected:
		//Member widgets:
		Gtk::Button btn0;
		Gtk::Button btn1;
		Gtk::Button btn2;
		Gtk::Label  label0;
		Gtk::Grid grid;
};

TestWin::TestWin() : btn0("Hello World"), btn1("short!"), btn2("very long text!"), label0("The label") {
	// Sets the border width of the window.
	// set_border_width(10);
	set_default_size(100, 100);
	set_title("Pixie 16");
	
	grid.attach(btn0,   0, 0);
	grid.attach(btn1,   1, 0);
	grid.attach(btn2,   0, 1);
	grid.attach(label0, 1, 1);
	
	label0.set_markup("<span font-weight=\"bold\" font-family=\"FreeMono\">fancy</span>");
	
	add(grid);
	
	grid.show_all();
}

TestWin::~TestWin() {}

style_t style = DEFAULT_STYLE();



int TextWidth(const char *text) {
	
}

int CalcSpacing(int elemWidth, int count, int total) {
	
}



void handleButtonEvent(button_t *button) {
	
}

void drawButton(button_t *button) {
	
}

void fmtNumber(char *buf, size_t buf_cap, double num, int len) {
	
}



void window_init() {
	
}

void window_main() {
	auto app = Gtk::Application::create();
	TestWin mainWindow;
	app->run(mainWindow);
}

void window_destroy() {
	
}

void window_redraw() {
	
}

bool window_poll() {
	
}
