
#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "window.h"
#include <px16.h>
#include <stdio.h>

extern bool debuggerOpen;

void debugger_init();
void debugger_event(XEvent event);
void debugger_loop();
void debugger_redraw();
void debugger_show();
void debugger_hide();
void debugger_close();

// Loads source code for the running binary from addr2line dump.
void debugger_load(FILE *fd);

#endif //DEBUGGER_H
