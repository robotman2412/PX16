
#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "window.h"

#ifdef __cplusplus
extern "C" {
#endif

extern bool debuggerOpen;

void debugger_init();
void debugger_event(XEvent event);
void debugger_redraw();
void debugger_show();
void debugger_hide();
void debugger_close();

#ifdef __cplusplus
}
#endif

#endif //DEBUGGER_H
