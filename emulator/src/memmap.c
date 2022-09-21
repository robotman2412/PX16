
#include "memmap.h"
#include <stdlib.h>
#include <stdio.h>

// Creates the memory map for EL BADGE.
void badge_mmap_create(badge_mmap *mmap, memmap *mem) {
	*mmap = (badge_mmap) {
		.rom     = NULL,
		.rom_len = 0,
		.ram     = malloc(sizeof(word) * 65536),
	};
	*mem = (memmap) {
		.mem_ctx   = mmap,
		.mem_read  = (word(*)(core*,word,bool,void*)) badge_mmap_read,
		.mem_write = (void(*)(core*,word,word,void*)) badge_mmap_write,
		.tick_ctx  = mmap,
		.pre_tick  = NULL,
		.post_tick = NULL,
	};
}

// Destroys the memory map for EL BADGE.
void badge_mmap_destroy(badge_mmap *mmap) {
	if (mmap->rom) {
		free((void *) mmap->rom);
	}
	free(mmap->ram);
}

// Loads a binary into ROM for EL BADGE.
void badge_load_rom(badge_mmap *mmap, char *filename) {
	FILE *fd = fopen(filename, "rb");
	if (!fd) return;
	
	fseek(fd, 0, SEEK_END);
	long len = ftell(fd);
	if (len & 1) {
		fclose(fd);
		return;
	}
	
	fseek(fd, 0, SEEK_SET);
	word *rom = malloc(len);
	for (word i = 0; i < len / 2; i++) {
		uint8_t tmp[2];
		fread(tmp, 1, 2, fd);
		rom[i] = tmp[0] | (tmp[1] << 8);
	}
	mmap->rom     = rom;
	mmap->rom_len = len / 2;
	
	fclose(fd);
}


// Read from EL BADGE.
word badge_mmap_read(core *cpu, word address, bool notouchy, badge_mmap *mmap) {
	if (address == 0xffff) {
		// MMIO length.
		return 0x0040;
	} else if (address < mmap->rom_len) {
		return mmap->rom[address];
	} else {
		return mmap->ram[address];
	}
}

// Write to EL BADGE.
void badge_mmap_write(core *cpu, word address, word value, badge_mmap *mmap) {
	// PUT RAM.
	mmap->ram[address] = value;
}
