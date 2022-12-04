
#ifndef RUNNER_H
#define RUNNER_H

#include <stdint.h>
#include <pthread.h>
#include "px16.h"
#include "main.h"

extern tickmode sim_mode;

void *runner_main(void *ignored);
void runner_join();
void runner_mutex_take();
void runner_mutex_release();

#endif // RUNNER_H
