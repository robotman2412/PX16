
#include "px16.h"
#include "main.h"
#include <string.h>

// Describes CU stages.
const char *cu_state_names[n_cu_states] = {
	"boot0", "boot1",
	"load0", "load1", "load2",
	"push",  "jsr",
	"addr",
	"act",   "mov",
	"pop",
	"intr0", "intr1", "intr2", "intr3", "intr4",
};


// Function for packing an instruction.
 __attribute__((pure))
 __attribute__((hot))
word pack_insn(instr insn) {
	return (insn.y & 1) << 15
		 | (insn.x & 7) << 12
		 | (insn.b & 7) <<  9
		 | (insn.a & 7) <<  6
		 | (insn.o & 63);
}

// Function for unpacking an instruction.
 __attribute__((pure))
 __attribute__((hot))
instr unpack_insn(word packed) {
	return (instr) {
		.y = (uint8_t) ((packed & 0x8000) >> 15),
		.x = (uint8_t) ((packed & 0x7000) >> 12),
		.b = (uint8_t) ((packed & 0x0e00) >>  9),
		.a = (uint8_t) ((packed & 0x01c0) >>  6),
		.o = (uint8_t)  (packed & 0x003f),
	};
}


// Decide the MOV condition for the given opcode.
 __attribute__((hot))
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
 __attribute__((hot))
word alu_act(core *cpu, word opcode, word a, word b, bool notouchy) {
	// Determine some parameters.
	bool  math1 = opcode >= OP_INC;
	opcode &= ~OFFS_MATH1;
	lword cin   = (opcode & OFFS_CC) ? !!(cpu->PF & FLAG_UCOUT)
				: (opcode != OP_ADD) ^ math1;
	opcode &= ~OFFS_CC;
	if (math1) b = 0;
	lword res, sres;
	bool scout = false;
	// Calculate some CRAP.
	switch (opcode) {
		case (OP_ADD):
			res = cin + a + b;
			break;
		case (OP_SUB):
		case (OP_CMP):
			res = cin + (word) a + (word) ~b;
			sres = cin + (lword) (a ^ 0x8000) + (lword) (b ^ 0x7fff);
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
			res = ((lword) a << 1) | cin;
			break;
		case (OP_SHR & ~OFFS_MATH1):
			res = (a >> 1) | ((lword) (a & 1) << 16) | (cin << 15);
			break;
		default:
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
void core_reset(core *cpu, bool hard) {
	memset(&cpu->state, 0, sizeof(cpu->state));
	cpu->state.boot_0 = true;
	cpu->PC = 0;
	cpu->insn_count = 0;
	cpu->jsr_count  = 0;
	cpu->irq_count  = 0;
	cpu->nmi_count  = 0;
	if (hard) {
		memset(cpu->regfile, 0, sizeof(cpu->regfile));
		cpu->imm0 = 0;
		cpu->imm1 = 0;
	}
}


// Simulates things that happen during the low time.
// Does not change register or memory states.
void core_pretick(core *cpu, memmap *mem) {
	instr run = cpu->current;
	
	// Set busses.
	cpu->par_bus_a = (run.a == 7) ? cpu->imm0 : cpu->regfile[run.a];
	cpu->par_bus_a = (run.b == 7) ? cpu->imm1 : cpu->regfile[run.b];
	
	// Set ALU part.
	word ret_bus = alu_act(cpu, cpu->current.o, cpu->par_bus_a, cpu->par_bus_b, true);
	
	// Calculate addr bus.
	
	// Read (notouchy) memory.
	bool read_mem = false;
	if (read_mem) {
		cpu->data_bus = mem->mem_read(cpu, mem, cpu->addr_bus, true, mem->mem_ctx);
	}
}

// Simulates the rising edge followed by the falling edge.
// Chenges register and/or memory states.
 __attribute__((hot))
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
 __attribute__((hot))
lword fast_tick(core *cpu, memmap *mem) {
	lword took = 0;
	// If we're on boot, finish that up.
	if (cpu->state.boot_0) {
		cpu->PC = mem->mem_read(cpu, mem, 2, false, mem->mem_ctx);
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
	
	if (mem->pre_tick) mem->pre_tick(cpu, mem, mem->tick_ctx);
	// Now we get to the fun stuff.
	// Unpack the instruction to run.
	instr run = unpack_insn(mem->mem_read(cpu, mem, cpu->PC++, false, mem->mem_ctx));
	took ++;
	cpu->insn_count ++;
	
	// Update the interrupt helper bit.
	cpu->intr_helper = cpu->PF & FLAG_IPROG;
	
	// Check for IMM0.
	if (run.a == PX_REG_IMM) {
		cpu->imm0 = mem->mem_read(cpu, mem, cpu->PC++, false, mem->mem_ctx);
		took ++;
	}
	// Check for IMM1.
	if (run.b == PX_REG_IMM) {
		cpu->imm1 = mem->mem_read(cpu, mem, cpu->PC++, false, mem->mem_ctx);
		took ++;
	}
	
	bool is_jsr  = run.o == (OFFS_MOV | COND_JSR) || run.o == (OFFS_LEA | COND_JSR);
	bool is_b_st = run.b == PX_REG_ST;
	
	// Check for push stage.
	if (is_jsr || (!run.y && run.x == PX_ADDR_MEM && run.a == PX_REG_ST)) {
		cpu->ST --;
		took ++;
	}
	// Check for jsr stage.
	if (is_jsr) {
		mem->mem_write(cpu, mem, cpu->ST, cpu->PC, mem->mem_ctx);
		took ++;
		cpu->jsr_count ++;
	}
	// Check for addr stage.
	if (run.x != PX_ADDR_IMM) {
		// Determine register for memory access.
		reg regno = (reg) (run.y ? run.b : run.a);
		word regval = regno != PX_REG_IMM ? cpu->regfile[regno]
					: (run.y ? cpu->imm1 : cpu->imm0);
		// Determine address.
		if (run.x == PX_ADDR_MEM) {
			cpu->AR = regval;
		} else {
			cpu->AR = regval + cpu->regfile[run.x];
		}
		// Read from memory.
		word data = mem->mem_read(cpu, mem, cpu->AR, false, mem->mem_ctx);
		// Write back to correct IMM register.
		if (run.y) {
			run.b = PX_REG_IMM;
			cpu->imm1 = data;
		} else {
			run.a = PX_REG_IMM;
			cpu->imm0 = data;
		}
		took ++;
		// Check for LEA.
		if (run.o >= OFFS_LEA) {
			if (decide_cond(cpu, run.o)) {
				cpu->regfile[run.a] = cpu->AR;
			}
			goto end;
		}
	}
	
	// Check for instruction type.
	if (run.o <= (OP_SHR | OFFS_CC)) {
		// Math instructions.
		word a = run.a == PX_REG_IMM ? cpu->imm0 : cpu->regfile[run.a];
		word b = run.b == PX_REG_IMM ? cpu->imm1 : cpu->regfile[run.b];
		word value = alu_act(cpu, run.o, a, b, false);
		if ((run.o & ~OFFS_CC & ~OFFS_MATH1) != OP_CMP) {
			if (run.a == PX_REG_IMM) {
				// Write to memory.
				mem->mem_write(cpu, mem, cpu->AR, value, mem->mem_ctx);
			} else {
				// Write to register.
				cpu->regfile[run.a] = value;
			}
		}
	} else {
		// MOV instructions.
		if (decide_cond(cpu, run.o)) {
			word value = run.b == PX_REG_IMM ? cpu->imm1 : cpu->regfile[run.b];
			// Check for carry extend condition.
			if (run.o == (OP_MOV | COND_CX)) {
				value = (value & 0x8000) ? 0xffff : 0x0000;
			}
			if (run.a == PX_REG_IMM) {
				// Write to memory.
				mem->mem_write(cpu, mem, cpu->AR, value, mem->mem_ctx);
			} else {
				// Write to register.
				cpu->regfile[run.a] = value;
			}
		}
	}
	took ++;
	
	// Check for pop.
	if (run.y && run.x == PX_ADDR_MEM && is_b_st) {
		cpu->ST ++;
		took ++;
	}
	
	bool is_nmi, is_isr, is_isr_allowed;
	end:
	// Interrupt handling.
	is_isr_allowed = !cpu->intr_helper && !(cpu->PF & FLAG_IPROG);
	is_nmi = mem->nmi;
	is_isr = (mem->nmi && (cpu->PF & FLAG_NMI)) || (mem->irq && (cpu->PF & FLAG_IRQ));
	if (is_isr && is_isr_allowed) {
		// Intr 0: Pre-decrement ST.
		cpu->ST --;
		// Intr 1: Push PC to stack.
		mem->mem_write(cpu, mem, cpu->ST, cpu->PC, mem->mem_ctx);
		// Intr 2: Pre-decrement ST.
		cpu->ST --;
		// Intr 3: Push PF to stack and set interrupt in progress flag
		mem->mem_write(cpu, mem, cpu->ST, cpu->PF, mem->mem_ctx);
		cpu->PF |= FLAG_IPROG;
		// Intr 4: Load interrupt vector.
		cpu->PC = mem->mem_read(cpu, mem, is_nmi ? 1 : 0, false, mem->mem_ctx);
		
		if (is_nmi) {
			cpu->nmi_count ++;
		} else {
			cpu->irq_count ++;
		}
		
		took += 5;
	}
	
	if (mem->post_tick) mem->post_tick(cpu, mem, mem->tick_ctx, took);
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

// Simulates as many cycles as possible until timeout is reached.
// Returns the real number of simulated cycles.
lword warp_ticks(core *cpu, memmap *mem, uint64_t timeout) {
	// Automatic calibration to get the best out of warp speed.
	static int calib = 1000000;
	lword real = 0;
	for (int i = 0; i < 10000 + calib; i++) {
		real += fast_tick(cpu, mem);
	}
	if (micros() < timeout - 250 && calib < 100000000) {
		calib += 10000;
	} else if (micros() > timeout + 250 && calib >= 1000) {
		calib -= 2500;
	}
	return real;
}

