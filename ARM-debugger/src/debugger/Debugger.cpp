#include "Debugger.h"

static void ARM::Debugger::setFlags(doubleword a, word b, word c, bool addone) {
	if(a & 0x80000000) SET_FLAG(NEGATIVE_FLAG);
	else CLEAR_FLAG(NEGATIVE_FLAG);

	if((a & 0xFFFFFFFF) == 0) SET_FLAG(ZERO_FLAG);
	else CLEAR_FLAG(ZERO_FLAG);

	if(a & 0x100000000) SET_FLAG(CARRY_FLAG);
	else CLEAR_FLAG(CARRY_FLAG);

	if(((b & 0x7FFFFFFF) + (c & 0x7FFFFFFF) + (addone ? 1 : 0)) & 0x80000000) SET_FLAG(OVERFLOW_FLAG);
	else CLEAR_FLAG(OVERFLOW_FLAG);
}

void ARM::Debugger::executeNextARM(word instr) {
	printf("[ARM]   %.08X: ", instr);

	byte condition = instr >> 28;

	struct decompiled { std::string instr, arg0; } decompiled;

	if (IS_SET(instr, 27)) {
		if (IS_SET(instr, 26)) {

		}
		else {
			if (IS_SET(instr, 25)) {
				bool link = IS_SET(instr, 24);
				word offset = (instr & 0x00FFFFFF) << 2;
				if (IS_SET(offset, 25)) offset |= 0xFC;

				if (link) state->lr = state->pc + 4;
				state->pc += 4 + offset; // 4 added already
			}
		}
	}
	else {
		if (IS_SET(instr, 26)) {
			if (IS_SET(instr, 25) && IS_SET(instr, 4)) {
				// UNDEF
			}
			else {
				bool imm = IS_SET(instr, 25);
				bool isPre = IS_SET(instr, 24);
				bool isUp = IS_SET(instr, 23);
				bool isByte = IS_SET(instr, 22);
				bool writeback = IS_SET(instr, 21);
				bool load = IS_SET(instr, 20);
				byte Rn = (instr >> 16) & 0xF;
				byte Rd = (instr >> 12) & 0xF;
				word offset = instr & 0xFFF;

				word address = state->readReg(Rn);

				if (imm || offset != 0) { // LDR/STR Rd, [Rn, Rm SHIFT/IMM]
					if (imm && Rn == 0xF) address += state->pc + offset + 4;
					else {
						if(imm) {
							word reg = state->readReg(offset & 0xF);
							// TODO shift....
							address += reg;
						}
						else address += offset;
					}
				}

				if(load) {
					if(isByte) state->writeReg(Rd, state->read8(address));
					else state->writeReg(Rd, state->read32(address));
				} else {
					if(isByte) state->write8(address, state->readReg(Rd));
					else state->write32(address, state->readReg(Rd));
				}
			}
		}
		else {
			if (((instr >> 4) & 0x9) == 0x9 && !IS_SET(instr, 25)) {

			}
			else if (((instr >> 16) & 0x3F) == 0x0F && ((instr >> 23) & 0x1F) == 0x02 && (instr & 0xFFF) == 0) {
				printf("MRS\n");
			}
			else if (((instr >> 12) & 0x3FF) == 0x29F && ((instr >> 23) & 0x1F) == 0x02 && (instr & 0xFF0) == 0) {
				if(IS_SET(instr, 22)) state->spsr = state->readReg(instr & 0xF);
				else state->cpsr = state->readReg(instr & 0xF);
			}
			else if (((instr >> 12) & 0x3FF) == 0x28F && ((instr >> 23) & 0x1B) == 0x02) {
				printf("MSR Flag bits only\n");
			}
			else if ((instr & 0x0FFFFFF0) == 0x012FFF10) {
				decompiled.instr = "BX";
				//decompiled.arg0 = register_names[instr & 0xF];

			}
			else {
				byte opcode = (instr >> 21) & 0xF;
				bool setConditionCodes = IS_SET(instr, 20);
				bool imm = IS_SET(instr, 25);
				byte Rn = (instr >> 16) & 0xF;
				byte Rd = (instr >> 12) & 0xF;
				word op2 = (instr & 0xFFF);

				if (imm) {
					word a = (op2 & 0xFF);
					byte rotate = (op2 >> 8) << 1;
					op2 = (a >> rotate) | (a << (32 - rotate));
				}
				else {
					word a = state->readReg(op2 & 0xF);

				}

				//decompiled.arg0 = register_names[Rd] + ", " + register_names[Rn];

				switch (opcode) {
				case 0x4:
					if (Rn == 0xF) state->writeReg(Rd, state->pc + 4 + op2);
					else state->writeReg(Rd, state->readReg(Rn) + 4 + op2);
					break;
				case 0x9:
					decompiled.instr = "TEQ";
					//decompiled.arg0 = register_names[Rn] + ", #" + to_hex(op2, 0);
					break;
				case 0x0A:
					decompiled.instr = "CMP";
					//decompiled.arg0 = register_names[Rn] + ", #" + to_hex(op2, 0);
					break;
				case 0xD:
					decompiled.instr = "MOV";
					//decompiled.arg0 = register_names[Rd] + ", #" + to_hex(op2, 0);
					break;
				default:
					printf("Data processing: %.02X\n", opcode);
					break;
				}
			}
		}
	}
}

void ARM::Debugger::executeNextTHUMB(halfword instr) {

}

void ARM::Debugger::executeNextInstruction() {
	printf("[DEBUGGER][%.08X]", state->pc);
	if (IS_SET(state->cpsr, 5)) {
		word instr = state->read16(state->pc);
		state->pc += 2;
		executeNextTHUMB(instr);
	}
	else {
		word addr = state->read32(state->pc);
		state->pc += 4;
		executeNextARM(instr);
	}
}
