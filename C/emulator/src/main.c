
#include "main.h"
#include "term.h"
#include <stdio.h>

int main(int argc, char **argv) {
	// Add the exit handler.
	if (atexit(exithandler)) {
		fputs("Could not register exit handler; aborting!\n", stderr);
		return -1;
	}
	
	// Set TTY mode to disable line buffering and echoing.
	system("stty cbreak -echo -isig");
	
	pos res = term_getpos();
	printf("Terminal pos was %d, %d.\n", res.x, res.y);
	return 0;
}

// Handler for program exit.
void exithandler() {
	// Restore TTY to sane.
	system("stty sane");
}
