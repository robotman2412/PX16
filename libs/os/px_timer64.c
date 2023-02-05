
#include "px_timer64.h"
#include "px_thread.h"

#define OVERFLOW_LENIENCY 1000
#define OVERFLOW_PREPARE (0xffffffffffffffffLLU - OVERFLOW_LENIENCY)

// 32-bit MSB of timer.
// Updated automatically by ISR.
uint32_t _px_timer_msb = 0;
// Current reason for timer limit.
timer_type_t _px_timer_type = TIMER_OVERFLOW;
// Next time of forced task switch.
// Updated by ISR or `_px_set_switch_delay`.
time_t _px_switch_time = 10000;
// Interval of forced task switch.
// Updated by `_px_set_switch_delay`.
time_t _px_switch_interval = 10000;
// First link in event queue.
timer_event_t *_px_timers = NULL;

// Main for timer thread.
static void timer_thread_main(void *ignored);



// ==== Initialisation ==== //

// Start the timer event handler thread.
// Called by the other initialisers.
void _px_timer_thread_init() {
	px_thd_cfg_t cfg = {
		.name       = "Timer Worker",
		.priority   = 0,
		.stack_size = 128,
	};
	px_thread_create(timer_thread_main, NULL, &cfg);
}



// ==== Preemptive multitasking ==== //

// Timer management routine upon timer limit reached.
void _px_timer_reached() {
	switch(_px_timer_type) {
		case TIMER_OVERFLOW:
			_px_timer_msb ++;
			break;
			
		case TIMER_EVENT:
			// TODO...
			break;
			
		case TIMER_TASK_SWITCH:
			px_yield_from_isr();
	}
}

// Update task switching interval and reschedule task switch accordingly.
void _px_set_switch_delay(time_t interval) {
	// Set timer0 limit.
	_px_switch_interval = interval;
	_px_switch_time = px_timer64() + interval;
	
	// Recalculate timer limit.
	px_calc_timer();
}



// ==== Timekeeping ==== //

// Main for timer thread.
static void timer_thread_main(void *ignored) {
	
}

// Recalculate timer limit.
void px_calc_timer() {
	time_t now = px_timer64();
	
	if (now >= OVERFLOW_PREPARE) {
		// If <= 1ms away, dedicate limit entirely to overflow detection.
		_px_timer_limit(0, TIMER_OVERFLOW);
		
	} else if (_px_timers && _px_timers->time < _px_switch_time) {
		// If timer happens before task switch.
		_px_timer_limit((uint32_t) _px_switch_time, TIMER_EVENT);
		
	} else if ((_px_switch_time>>32) < _px_timer_msb) {
		// If task switch happens before overflow.
		_px_timer_limit((uint32_t) _px_switch_time, TIMER_TASK_SWITCH);
		
	} else {
		// Overflow is the first thing that will happen.
		_px_timer_limit(0, TIMER_OVERFLOW);
	}
}
