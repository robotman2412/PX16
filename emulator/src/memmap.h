
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
};

// Creates the memory map for EL BADGE.
void badge_mmap_create(badge_mmap *mmap, memmap *mem);
// Destroys the memory map for EL BADGE.
void badge_mmap_destroy(badge_mmap *mmap);
// Loads a binary into ROM for EL BADGE.
void badge_load_rom(badge_mmap *mmap, char *filename);

// Read from EL BADGE.
word badge_mmap_read(core *cpu, word address, bool notouchy, badge_mmap *ctx);
// Write to EL BADGE.
void badge_mmap_write(core *cpu, word address, word value, badge_mmap *ctx);

#endif //MEMMAP_H
