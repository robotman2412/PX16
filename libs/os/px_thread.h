
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ==== Types ==== //

#define PX_THD_CAPACITY 8
#define PX_THD_SELF 0xffff

#define PX_THD_STATIC_STACK 0x0001
#define PX_THD_DETACHED     0x0002

// Status for a thread.
typedef enum {
	// Vacant thread slot.
	THREAD_VACANT,
	// Paused.
	THREAD_PAUSED,
	// Dead, but not yet cleaned up.
	THREAD_DEAD,
	// Blocked by mutex.
	THREAD_BLOCK_MUTEX,
	// Blocked by I/O.
	THREAD_BLOCK_IO,
	// Joining another thread.
	THREAD_BLOCK_JOIN,
} px_thd_status_t;

// Context local to a thread.
typedef struct {
	// Stack pointer of the thread.
	// Avoid moving this field; used by `px_thread_wake` in ASM.
	size_t   stack_ptr;
	// Maximum stack pointer; base address of ST.
	size_t   stack_base;
	// Size of stack.
	size_t   stack_size;
	// List of status and property flags.
	uint16_t flags;
	// Execution state of thread.
	px_thd_status_t status;
	// Which object, if any, is blocking the thread.
	void    *block;
	// Name of the thread, if any.
	char    *name;
	// Priority of the thread, 0 is default.
	int      priority;
	// First time at which the thread has ran code.
	uint64_t start_time;
	// Total CPU time used by the thread so far.
	uint64_t used_time;
} px_thd_t;

// Context used to create a thread.
typedef struct {
	// Size of stack.
	size_t      stack_size;
	// Name of the thread (optional; auto-generated when NULL).
	const char *name;
	// Priority of the thread, 0 is default.
	int         priority;
} px_thd_cfg_t;

// Thread main function type.
typedef void(*px_thd_fn_t)(void *arg);

// Thread handle.
typedef size_t px_thd_handle_t;

// ==== Globals ==== //

// The currently active thread.
// Avoid renaming the field; used by `px_thread_wake` in ASM.
extern px_thd_t *_px_current_thread;
// All threads.

// ==== Context switching ==== //

// Disables interrupts.
static inline void px_disable_irq() {
	asm volatile(
		"AND PF, 0xfffd"
	);
}
// Enables interrupts.
static inline void px_enable_irq() {
	asm volatile(
		"OR PF, 0x0002"
	);
}

// Switches contexts to the given context.
// Sets _px_current_thread after switch.
void px_thread_wake(px_thd_t *thread);
// Forces a context switch to happen.
void px_yield();
// Forces a context switch to happen; ISR safe gauranteed.
void px_yield_from_isr();

// ==== Thread management ==== //

// Initialise threading system and start current PC as a thread.
void px_enter_threading(px_thd_cfg_t *cfg);
// Create new thread from function.
// Upon return of it's main function, the thread will be destroyed.
void px_thread_create(px_thd_fn_t main, void *args, px_thd_cfg_t *cfg);
// Join a thread; wait for it to finish executing.
// When the thread exits, its resources are freed.
void px_thread_join(px_thd_handle_t thread);
// Indicate that the thread will never be joined.
// Upon death, thread immediately gets cleaned up.
void px_thread_detach(px_thd_handle_t thread);
// Terminate a thread prematurely.
// This often has resource allocation implications.
// The subject thread, or current thread if PX_THD_SELF, is terminated and cleaned up.
void px_thread_kill(px_thd_handle_t thread);

#ifdef __cplusplus
}
#endif
