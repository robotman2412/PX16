
#ifndef PX_THREAD_H
#define PX_THREAD_H

// ==== Context switching ==== //

// Stores the current thread context.
// This happens before thread switch.
void px_store_ctx(px_ctx_t *ctx);
// Loads the current thread context.
// This happens after thread switch.
void px_load_ctx(px_ctx_t *ctx);

#endif //PX_THREAD_H
