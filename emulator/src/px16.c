
#include "px16.h"

// Describes CU stages.
const char *cu_state_names[n_cu_states] = {
	"boot0", "boot1",
	"load0", "load1", "load2",
	"push",  "jsr",
	"addr",
	"act",   "mov",
	"pop",
};


// Function for packing an instruction.
 __attribute__((pure))
word pack_insn(instr insn) {
	return (insn.y & 1) << 15
		 | (insn.x & 7) << 12
		 | (insn.b & 7) <<  9
		 | (insn.a & 7) <<  6
		 | (insn.o & 63);
}

// Function for unpacking an instruction.
 __attribute__((pure))
instr unpack_insn(word packed) {
	return (instr) {
		.y = (packed & 0x8000) >> 15,
		.x = (packed & 0x7000) >> 12,
		.b = (packed & 0x0e00) >>  9,
		.a = (packed & 0x01c0) >>  6,
		.o = (packed & 0x003f),
	};
}


// Decide the MOV condition for the given opcode.
bool decide_cond(core *cpu, word opcode) {
	opcode &= 017;
	bool scout = cpu->PF & FLAG_SCOUT;
	bool ucout = cpu->PF & FLAG_UCOUT;
	bool zero  = cpu->PF & FLAG_ZERO;
	switch (opcode) {
		// Unsigned less than.
		case (COND_ULT):
			return !ucout && !zero;
		// Unsigned greater than.
		case (COND_UGT):
			return ucout && !zero;
		// Signed less than.
		case (COND_SLT):
			return !scout && !zero;
		// Signed greater than.
		case (COND_SGT):
			return scout && !zero;
		// Equal.
		case (COND_EQ):
			return zero;
		// Unsigned carry set.
		case (COND_CS):
			return ucout;
		// Unsigned greater than or equal.
		case (COND_UGE):
			return ucout || zero;
		// Unsigned less than or equal.
		case (COND_ULE):
			return !ucout || zero;
		// Signed greater than or equal.
		case (COND_SGE):
			return scout || zero;
		// Signed less than or equal.
		case (COND_SLE):
			return !scout || zero;
		// Not equal.
		case (COND_NE):
			return !zero;
		// Unsigned carry not set.
		case (COND_CC):
			return !ucout;
		default:
			return true;
	}
}

// Calculate the ALU result for the ACT stage.
// Writes to PF unless notouchy is 1.
word alu_act(core *cpu, word opcode, word a, word b, bool notouchy) {
	// Determine some parameters.
	bool  math1 = opcode >= OP_INC;
	opcode &= ~OFFS_MATH1;
	lword cin   = (opcode & OFFS_CC) ? !!(cpu->PF & FLAG_UCOUT)
				: (opcode != OP_ADD) ^ math1;
	opcode &= ~OFFS_CC;
	if (math1) b = 0;
	lword res;
	bool scout = false;
	// Calculate some CRAP.
	switch (opcode) {
		case (OP_ADD):
			res = cin + a + b;
			break;
		case (OP_SUB):
		case (OP_CMP):
			res = cin + (word) a + (word) ~b;
			lword sres = cin + (lword) (a ^ 0x8000) + (lword) (b ^ 0x7fff);
			scout = sres & 0x10000;
			break;
		case (OP_AND):
			res = a & b;
			break;
		case (OP_OR):
			res = a | b;
			break;
		case (OP_XOR):
			res = a ^ b;
			break;
		case (OP_SHL & ~OFFS_MATH1):
			res = (lword) a << (lword) 1;
			break;
		case (OP_SHR & ~OFFS_MATH1):
			res = (a >> 1) | (((lword) a & (lword) 1) << 16);
			break;
	}
	// Set PF.
	if (!notouchy) {
		bool cout = res & 0x10000;
		bool zero = !(res & 0xffff);
		cpu->PF = (cpu->PF & ~FLAG_SCOUT & ~FLAG_UCOUT & ~FLAG_ZERO)
				| scout * FLAG_SCOUT | cout * FLAG_UCOUT | zero * FLAG_ZERO;
	}
	// Done!
	return res;
}


// Resets the core in the way defined by the spec.
// This means only resetting the CU and PC.
void core_reset(core *cpu) {
	cpu->state = (core_cu) {
		.boot_0 = 1,
		.boot_1 = 0,
		.load_0 = 0,
		.load_1 = 0,
		.load_2 = 0,
		.push   = 0,
		.jsr    = 0,
		.addr   = 0,
		.act    = 0,
		.mov    = 0,
		.pop    = 0,
	};
	cpu->PC = 0;
}


// Simulates things that happen during the low time.
// Does not change register or memory states.
void core_pretick(core *cpu, memmap *mem) {
	instr run = cpu->current;
	
	// Set busses.
	cpu->par_bus_a = (run.a == 7) ? cpu->imm0 : cpu->regfile[run.a];
	cpu->par_bus_a = (run.b == 7) ? cpu->imm1 : cpu->regfile[run.b];
	
	// TODO: Set ALU part.
	
	// CRACK addr bus.
	
	// Read (notouchy) memory.
	bool read_mem = false;
	if (read_mem) {
		cpu->data_bus = mem->mem_read(cpu, cpu->addr_bus, true, mem->mem_ctx);
	}
}

// Simulates the rising edge followed by the falling edge.
// Chenges register and/or memory states.
void core_posttick(core *cpu, memmap *mem) {}


// Simulates one full clock cycle of the core.
void core_tick(core *cpu, memmap *mem) {
	core_pretick(cpu, mem);
	core_posttick(cpu, mem);
	core_pretick(cpu, mem);
}

// Simulates exactly cycles full clock cycles of the core.
void core_ticks(core *cpu, memmap *mem, lword cycles) {
	core_pretick(cpu, mem);
	for (; cycles; cycles --) {
		core_posttick(cpu, mem);
		core_pretick(cpu, mem);
	}
}


// Simulates a full instruction instead of a number of cycles.
// Returns the number of cycles the instruction takes in reality.
lword fast_tick(core *cpu, memmap *mem) {
	lword took = 0;
	// If we're on boot, finish that up.
	if (cpu->state.boot_0) {
		cpu->PC = mem->mem_read(cpu, 2, false, mem->mem_ctx);
		took = 2;
		cpu->state.boot_0 = 0;
		cpu->state.load_0 = 1;
	} else if (cpu->state.boot_1) {
		core_tick(cpu, mem);
		took ++;
	}
	// If we're midway through executing, finish that and return.
	// if (!cpu->state.load_0) {
	// 	while (!cpu->state.load_0) {
	// 		core_tick(cpu, mem);
	// 		took ++;
	// 	}
	// 	return took;
	// }
	
	if (mem->pre_tick) mem->pre_tick(cpu, mem->tick_ctx);
	// Now we get to the fun stuff.
	// Unpack the instruction to run.
	instr run = unpack_insn(mem->mem_read(cpu, cpu->PC++, false, mem->mem_ctx));
	took ++;
	
	// Check for IMM0.
	if (run.a == REG_IMM) {
		cpu->imm0 = mem->mem_read(cpu, cpu->PC++, false, mem->mem_ctx);
		took ++;
	}
	// Check for IMM1.
	if (run.b == REG_IMM) {
		cpu->imm1 = mem->mem_read(cpu, cpu->PC++, false, mem->mem_ctx);
		took ++;
	}
	bool is_jsr = run.o == (OFFS_MOV | COND_JSR) || run.o == (OFFS_LEA | COND_JSR);
	// Check for push stage.
	if (is_jsr || (!run.y && run.x == ADDR_MEM && run.a == REG_ST)) {
		cpu->ST --;
		took ++;
	}
	// Check for jsr stage.
	if (is_jsr) {
		mem->mem_write(cpu, cpu->ST, cpu->PC, mem->mem_ctx);
		took ++;
	}
	// Check for addr stage.
	if (run.x != ADDR_IMM) {
		reg regno = run.y ? run.b : run.a;
		word regval = regno != REG_IMM ? cpu->regfile[regno]
					: (run.y ? cpu->imm1 : cpu->imm0);
		if (run.x == ADDR_MEM) {
			cpu->AR = regval;
		} else {
			cpu->AR = regval + cpu->regfile[run.x];
		}
		word data = mem->mem_read(cpu, cpu->AR, false, mem->mem_ctx);
		if (run.y) {
			run.b = REG_IMM;
			cpu->imm1 = data;
		} else {
			run.a = REG_IMM;
			cpu->imm0 = data;
		}
		took ++;
		// Check for LEA.
		if (run.o >= OFFS_LEA) {
			goto end;
		}
	}
	// Check for instruction type.
	if (run.o <= (OP_SHR | OFFS_CC)) {
		// Math instructions.
		word a = run.a == REG_IMM ? cpu->imm0 : cpu->regfile[run.a];
		word b = run.b == REG_IMM ? cpu->imm1 : cpu->regfile[run.b];
		word value = alu_act(cpu, run.o, a, b, false);
		if (run.a == REG_IMM) {
			// Write to memory.
			mem->mem_write(cpu, cpu->AR, value, mem->mem_ctx);
		} else {
			// Write to register.
			cpu->regfile[run.a] = value;
		}
	} else {
		// MOV instructions.
		if (decide_cond(cpu, run.o)) {
			word value = run.b == REG_IMM ? cpu->imm1 : cpu->regfile[run.b];
			// Check for CX.
			if (run.o == OP_MOV | COND_CX) {
				value = (value & 0x8000) ? 0xffff : 0x0000;
			}
			if (run.a == REG_IMM) {
				// Write to memory.
				mem->mem_write(cpu, cpu->AR, value, mem->mem_ctx);
			} else {
				// Write to register.
				cpu->regfile[run.a] = value;
			}
		}
	}
	took ++;
	// Check for pop.
	if (run.y && run.x == ADDR_MEM && run.a == REG_ST) {
		cpu->ST ++;
		took ++;
	}
	end:
	if (mem->post_tick) mem->post_tick(cpu, mem->tick_ctx);
	return took;
}

// Simulates at least cycles full clock cycles worth of instructions.
// Uses fast_tick internally, which makes this usually take a cycle or two more.
// Returns the real number of simulated cycles.
lword fast_ticks(core *cpu, memmap *mem, lword cycles) {
	lword real = 0;
	do {
		real += fast_tick(cpu, mem);
	} while (real < cycles);
	return real;
}

