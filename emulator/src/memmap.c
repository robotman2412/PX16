
#include "memmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Creates the memory map for EL BADGE.
void badge_mmap_create(badge_mmap *mmap, memmap *mem) {
	*mmap = (badge_mmap) {
		.rom          = NULL,
		.rom_len      = 0,
		.ram          = malloc(sizeof(word) * 65536),
		.timer0_en    = false,
		.timer0_irq   = false,
		.timer0_nmi   = false,
		.timer0_value = 0,
		.timer0_int   = 0,
		.timer0_limit = 0,
		.timer0_trig  = false,
	};
	memset(mmap->ram, 0, sizeof(word) * 65536);
	*mem = (memmap) {
		.mem_ctx   = mmap,
		.mem_read  = (word(*)(core*,word,bool,void*)) badge_mmap_read,
		.mem_write = (void(*)(core*,word,word,void*)) badge_mmap_write,
		.tick_ctx  = mmap,
		.pre_tick  = NULL,
		.post_tick = (void(*)(core*,void*,lword)) badge_mmap_posttick,
		.reset     = NULL,
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
	if (address >= 0xffe0) {
		if (address == 0xffff) {
			// MMIO length.
			return 0x0040;
		} else if (address == 0xfffe) {
			// Power / timer mode.
			return mmap->timer0_en
				| (mmap->timer0_nmi << 1)
				| (mmap->timer0_irq << 2);
		} else if (address == 0xfffb) {
			// Timer 0 high.
			return mmap->timer0_int >> 16;
		} else if (address == 0xfffa) {
			// Timer 0 low.
			return mmap->timer0_int;
		} else if (address == 0xfff9) {
			// Timer 0 high.
			return mmap->timer0_limit >> 16;
		} else if (address == 0xfff8) {
			// Timer 0 low.
			return mmap->timer0_limit;
		}
	} else if (address < mmap->rom_len) {
		return mmap->rom[address];
	} else {
		return mmap->ram[address];
	}
}

// Write to EL BADGE.
void badge_mmap_write(core *cpu, word address, word value, badge_mmap *mmap) {
	if (address >= 0xffe0) {
		// Check MMIO things.
		
		if (address == 0xfffe) {
			// Timer mode.
			mmap->timer0_en  = value & 0x0001;
			mmap->timer0_nmi = value & 0x0004;
			mmap->timer0_irq = value & 0x0002;
			
			// Timer triggers.
			lword timer0_value = mmap->timer0_value;
			if (value & 0x0100) {
				// Write intermediate to timer value.
				mmap->timer0_value = mmap->timer0_int;
			}
			if (value & 0x0200) {
				// Write timer value to intermediate.
				mmap->timer0_int = timer0_value;
			}
			if (value & 0x0400) {
				// Acknowledge timer interrupt.
				mmap->timer0_trig = false;
			}
		} else if (address == 0xfffb) {
			// Timer 0 high.
			mmap->timer0_int   = (mmap->timer0_int & 0x0000ffff) | (value << 16);
		} else if (address == 0xfffa) {
			// Timer 0 low.
			mmap->timer0_int   = (mmap->timer0_int & 0xffff0000) | value;
		} else if (address == 0xfff9) {
			// Timer 0 high.
			mmap->timer0_limit = (mmap->timer0_limit & 0x0000ffff) | (value << 16);
		} else if (address == 0xfff8) {
			// Timer 0 low.
			mmap->timer0_limit = (mmap->timer0_limit & 0xffff0000) | value;
		}
		
	} else {
		// PUT RAM.
		mmap->ram[address] = value;
	}
}

// Post TICK (used for timer).
void badge_mmap_posttick(core *cpu, badge_mmap *mmap, lword cycles) {
	if (mmap->timer0_en) {
		// Increment the timer.
		lword pre = mmap->timer0_value;
		mmap->timer0_value = pre + cycles;
		
		// Check for trigger.
		bool trigger;
		if (mmap->timer0_value < pre) {
			trigger = mmap->timer0_limit <= mmap->timer0_value || mmap->timer0_limit >= pre;
		} else {
			trigger = mmap->timer0_limit >= pre && mmap->timer0_limit <= mmap->timer0_value;
		}
		
		// Update some stuff.
		mmap->timer0_int |= trigger;
	}
}
