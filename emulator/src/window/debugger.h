
#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "window.h"

#ifdef __cplusplus
extern "C" {
#endif

extern bool debuggerOpen;

void debugger_init();
bool debugger_poll();

#ifdef __cplusplus
}
#endif

#endif //DEBUGGER_H
