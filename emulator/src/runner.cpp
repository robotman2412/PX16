
#include "runner.h"
#include <unistd.h>

static uint64_t next_time = 0;
static uint64_t prev_time = 0;
static int64_t  too_fast  = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


 __attribute__((hot))
void runner_cycle() {
	bool warp_speed_tmp = warp_speed;
	
	// Simulate.
	uint64_t tick_count;
	if (warp_speed) {
		tick_count = warp_ticks(&cpu, &mem, micros() + 20000);
	} else if (sim_ticks > too_fast) {
		tick_count = fast_ticks(&cpu, &mem, sim_ticks - too_fast);
	} else {
		tick_count = 0;
	}
	sim_total_ticks += tick_count;
	too_fast = warp_speed ? 0 : tick_count - sim_ticks + too_fast;
	
	// Set next wakeup time.
	uint64_t now = micros();
	next_time += sim_us_delay;
	
	// Measure speed.
	int64_t delta = now - prev_time;
	double next_hertz = 1000000.0 / (double) delta * (double) tick_count;
	rolling_avg[rolling_idx] = next_hertz;
	rolling_idx = (rolling_idx + 1) % N_ROLLING_AVG;
	measured_hertz = 0;
	for (int i = 0; i < N_ROLLING_AVG; i++) {
		measured_hertz += rolling_avg[i] / N_ROLLING_AVG;
	}
	
	prev_time = now;
	if (warp_speed_tmp || next_time < now - 4*sim_us_delay) next_time = now;
	if (warp_speed_tmp) too_fast = 0;
}

void *runner_main(void *ignored) {
	
	pthread_mutex_init(&mutex, NULL);
	
	next_time = micros() + sim_us_delay;
	prev_time = micros();
	
	while (true) {
		if (running) {
			runner_mutex_take();
			int64_t wait = next_time - micros();
			while (wait > 0) {
				if (wait > 10000) usleep(10000);
				else usleep(wait);
				wait = next_time - micros();
			}
			runner_cycle();
			runner_mutex_release();
		} else {
			usleep(50000);
			too_fast = 0;
		}
	}
	
	return NULL;
}

void runner_join() {
	runner_mutex_take();
	runner_mutex_release();
}

void runner_mutex_take() {
	pthread_mutex_lock(&mutex);
}

void runner_mutex_release() {
	pthread_mutex_unlock(&mutex);
}

tickmode sim_mode = TICK_NORMAL;
