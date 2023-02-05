
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ==== Types ==== //

// Type used for time calculations.
typedef uint64_t time_t;
// Type used as event handle.
typedef uint32_t timer_handle_t;

// Callback function for timer event.
typedef void(*timer_cb_t)(time_t time, void *ctx);
// An event in the timer's queue.
typedef struct timer_event timer_event_t;
struct timer_event {
	// Previous link, if any.
	timer_event_t *prev;
	// Next link, if any.
	timer_event_t *next;
	
	// Timestamp of event.
	time_t time;
	
	// Callback function.
	timer_cb_t callback;
	// Callback arguments.
	void      *args;
};

// Type of reason timer limit is currently set for.
typedef enum {
	// Timer32 overflow.
	TIMER_OVERFLOW,
	// Preemptive task switch.
	TIMER_TASK_SWITCH,
	// Timer event queue.
	TIMER_EVENT,
} timer_type_t;

// ==== Globals ==== //

// 32-bit MSB of timer.
// Updated automatically by ISR.
extern uint32_t _px_timer_msb;
// Current reason for timer limit.
extern timer_type_t _px_timer_type;
// Next time of forced task switch.
// Updated by ISR or `_px_set_switch_delay`.
extern time_t _px_switch_time;
// Interval of forced task switch.
// Updated by `_px_set_switch_delay`.
extern time_t _px_switch_interval;
// First link in event queue.
extern timer_event_t *_px_timers;

// ==== Initialisation ==== //

// Initialise timer system, start counting from 0.
// Timer should be started after threading.
void px_timer_init_0();
// Start the timer event handler thread.
// Called by the other initialisers.
void _px_timer_thread_init();

// ==== Preemptive multitasking ==== //

// Timer management routine upon timer limit reached.
void _px_timer_reached();
// Update task switching interval and reschedule task switch accordingly.
void _px_set_switch_delay(time_t interval);

// ==== Timekeeping ==== //

// Set timer limit.
void _px_timer_limit(uint32_t limit, timer_type_t type);
// Recalculate timer limit.
void px_calc_timer();
// Read timer microseconds (32-bit; prone to overflow every 71 minutes).
uint32_t px_time32();
// Read timer microseconds (64-bit).
uint64_t px_time64();

// Schedule an event.

#ifdef __cplusplus
}
#endif
