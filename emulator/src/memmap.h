
#ifndef MEMMAP_H
#define MEMMAP_H

struct s_badge_mmap;

typedef struct s_badge_mmap badge_mmap;

#include "px16.h"

struct s_badge_mmap {
	/* ==== Physical memory ==== */
	// The ROM data.
	const word *rom;
	// The size of the ROM data.
	word  rom_len;
	// The RAM data.
	// Must be at least 655536 words.
	word *ram;
	/* ==== Memory mapped I/O ==== */
	// Timer mode.
	bool  timer0_en, timer0_irq, timer0_nmi;
	// Timer interrupt requests.
	bool  timer0_trig;
	// Timer 0 value.
	lword timer0_value;
	// Timer 0 intermediate register.
	lword timer0_int;
	// Timer 0 limit.
	lword timer0_limit;
};

// Creates the memory map for EL BADGE.
void badge_mmap_create(badge_mmap *mmap, memmap *mem);
// Destroys the memory map for EL BADGE.
void badge_mmap_destroy(badge_mmap *mmap);
// Loads a binary into ROM for EL BADGE.
void badge_load_rom(badge_mmap *mmap, char *filename);

// Read from EL BADGE.
word badge_mmap_read(core *cpu, memmap *mem, word address, bool notouchy, badge_mmap *ctx);
// Write to EL BADGE.
void badge_mmap_write(core *cpu, memmap *mem, word address, word value, badge_mmap *ctx);
// Get type of MEM for EL BADGE.
memtype badge_mmap_gettype(core *cpu, memmap *mem, word address, badge_mmap *ctx);
// Post TICK (used for timer).
void badge_mmap_posttick(core *cpu, memmap *mem, badge_mmap *ctx, lword cycles);
// RESET.
void badge_mmap_reset(core *cpu, memmap *mem, badge_mmap *ctx, bool hard);

#endif //MEMMAP_H
