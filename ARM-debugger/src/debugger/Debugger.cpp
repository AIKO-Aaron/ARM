#include "Debugger.h"

void ARM::Debugger::setFlags(dword b, dword c, bool addone) {
	dword a = (dword)b + (dword)c + ((dword)addone ? 1LL : 0LL);

	if(a & 0x80000000) SET_FLAG(FLAG_NEGATIVE);
	else CLEAR_FLAG(FLAG_NEGATIVE);

	if((a & 0xFFFFFFFF) == 0) SET_FLAG(FLAG_ZERO);
	else CLEAR_FLAG(FLAG_ZERO);

	if(a & 0x100000000) SET_FLAG(FLAG_CARRY);
	else CLEAR_FLAG(FLAG_CARRY);

	if(((b & 0x7FFFFFFF) + (c & 0x7FFFFFFF) + (addone ? 1 : 0)) & 0x80000000) SET_FLAG(FLAG_OVERFLOW);
	else CLEAR_FLAG(FLAG_OVERFLOW);
}

void ARM::Debugger::executeNextARM(word instr) {
	printf("[ARM]   %.08X: ", instr);

	byte condition = (instr >> 28) & 0xF;

	switch (condition) {
	case 0x0: if (state->cpsr & FLAG_ZERO) break; else return;
	case 0x1: if (state->cpsr & FLAG_ZERO) return; else break;
	case 0x2: if (state->cpsr & FLAG_CARRY) break; else return;
	case 0x3: if (state->cpsr & FLAG_CARRY) return; else break;
	case 0x4: if (state->cpsr & FLAG_NEGATIVE) break; else return;
	case 0x5: if (state->cpsr & FLAG_NEGATIVE) return; else break;
	case 0x6: if (state->cpsr & FLAG_OVERFLOW) break; else return;
	case 0x7: if (state->cpsr & FLAG_OVERFLOW) return; else break;
	case 0x8: if ((state->cpsr & FLAG_CARRY) && !(state->cpsr & FLAG_CARRY)) break; else return;
	case 0x9: if (!(state->cpsr & FLAG_CARRY) && (state->cpsr & FLAG_CARRY)) break; else return;
	case 0xA: if (((state->cpsr & FLAG_NEGATIVE) >> 3) == (state->cpsr & FLAG_OVERFLOW)) break; else return;
	case 0xB: if (((state->cpsr & FLAG_NEGATIVE) >> 3) == (state->cpsr & FLAG_OVERFLOW)) return; else break;
	case 0xC: if (((state->cpsr & FLAG_NEGATIVE) >> 3) == (state->cpsr & FLAG_OVERFLOW) && !(state->cpsr & FLAG_ZERO)) break; else return;
	case 0xD: if (((state->cpsr & FLAG_NEGATIVE) >> 3) == (state->cpsr & FLAG_OVERFLOW) && !(state->cpsr & FLAG_ZERO)) return; else break;
	case 0xE: break; // Always
	case 0xF: printf("This condition should not happen...\n"); return;
	}

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
					if (!imm && Rn == 0xF) address = state->pc + offset + 4;
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
				state->writeReg((instr >> 12) & 0xF, IS_SET(instr, 22) ? state->spsr : state->cpsr);
			}
			else if (((instr >> 12) & 0x3FF) == 0x29F && ((instr >> 23) & 0x1F) == 0x02 && (instr & 0xFF0) == 0) {
				if(IS_SET(instr, 22)) state->spsr = state->readReg(instr & 0xF);
				else {
					word newCpsr = state->readReg(instr & 0xF);
					state->setMode(newCpsr & 0x1F);
					state->cpsr = newCpsr;
				}
			}
			else if (((instr >> 12) & 0x3FF) == 0x28F && ((instr >> 23) & 0x1B) == 0x02) {
				printf("MSR Flag bits only\n");
			}
			else if ((instr & 0x0FFFFFF0) == 0x012FFF10) {
				state->pc = state->readReg(instr & 0xF) & 0xFFFFFFFE;
				
				if (state->readReg(instr & 0xF) & 1) state->cpsr |= 0x00000020;
				else state->cpsr &= ~0x00000020;
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
					setFlags(state->readReg(Rn) ^ op2, 0, false);
					break;
				case 0x0A:
					setFlags(state->readReg(Rn), ~(dword)op2, true);
					break;
				case 0xC:
					state->writeReg(Rd, state->readReg(Rn) | op2);
					if (setConditionCodes) setFlags(state->readReg(Rn) | op2, 0, false);
					break;
				case 0xD:
					state->writeReg(Rd, op2);
					break;
				default:
					printf("Data processing: %.02X\n", opcode);
					break;
				}
			}
		}
	}
}

void ARM::Debugger::executeNextTHUMB(hword instr) {
	struct decompiled { std::string instr, arg0; } decompiled;

	if ((instr >> 13) == 0x0) {
		if ((instr >> 11) == 0x3) {
			decompiled.instr = IS_SET(instr, 9) ? "SUBS" : "ADDS";

			word offset = 0;
			if(IS_SET(instr, 10)) offset += ((instr >> 6) & 0x7);
			else offset += state->readReg((instr >> 6) & 0x7);

			if (IS_SET(instr, 9)) {
				setFlags(state->readReg((instr >> 3) & 0x7), ~offset, true);
				state->writeReg(instr & 0x7, state->readReg((instr >> 3) & 0x7) - offset);
			}
			else {
				setFlags(state->readReg((instr >> 3) & 0x7), offset, false);
				state->writeReg(instr & 0x7, state->readReg((instr >> 3) & 0x7) + offset);
			}
			//decompiled.arg0 = register_names[instr & 0x7] + ", " + register_names[(instr >> 3) & 0x7] + ", ";
			//if (IS_SET(instr, 10)) decompiled.arg0 += "#" + to_hex((instr >> 6) & 0x7, 0);
			//else decompiled.arg0 += register_names[(instr >> 6) & 0x7];
		}
		else {
			byte Rd = (instr) & 0x7;
			byte Rs = (instr >> 3) & 0x7;
			byte offset = (instr >> 6) & 0x1F;
			//decompiled.arg0 = register_names[Rd] + ", " + register_names[Rs] + ", #" + to_hex((word)offset, 0);

			byte opcode = (instr >> 11) & 3;
			switch (opcode) {
			case 0x0:
				decompiled.instr = "LSLS";
				break;
			case 0x1:
				decompiled.instr = "LSRS";
				break;
			case 0x2:
				decompiled.instr = "ASRS";
				break;
			}
		}
	}
	else if ((instr >> 13) == 0x1) {
		byte opcode = (instr >> 11) & 3;
		byte Rd = (instr >> 8) & 0x7;
		byte offset = (instr & 0xFF);
		//decompiled.arg0 = register_names[Rd] + ", #" + to_hex((word)offset, 0);

		switch (opcode) {
		case 0x0:
			state->writeReg(Rd, offset);
			setFlags(offset, 0, false);
			break;
		case 0x1:
			decompiled.instr = "CMP";
			break;
		case 0x2:
			setFlags(offset, state->readReg(Rd), false);
			state->writeReg(Rd, state->readReg(Rd) + offset);
			break;
		case 0x3:
			decompiled.instr = "SUBS";
			break;
		}
	}
	else if ((instr >> 13) == 0x2) {
		if ((instr >> 10) == 0x10) {
			byte opcode = (instr >> 6) & 0xF;

			//decompiled.arg0 = register_names[instr & 0x7] + ", " + register_names[(instr >> 3) & 0x7];
			switch (opcode) {
			case 0x0:
				decompiled.instr = "ANDS";
				break;
			case 0x1:
				decompiled.instr = "EORS";
				break;
			case 0x2:
				decompiled.instr = "LSLS";
				break;
			case 0x3:
				decompiled.instr = "LSRS";
				break;
			case 0x4:
				decompiled.instr = "ASRS";
				break;
			case 0x5:
				decompiled.instr = "ADCS";
				break;
			case 0x6:
				decompiled.instr = "SBCS";
				break;
			case 0x7:
				decompiled.instr = "RORS";
				break;
			case 0x8:
				decompiled.instr = "TST";
				break;
			case 0x9:
				decompiled.instr = "NEGS";
				break;
			case 0xA:
				decompiled.instr = "CMP";
				break;
			case 0xB:
				decompiled.instr = "CMN";
				break;
			case 0xC:
				decompiled.instr = "ORRS";
				break;
			case 0xD:
				decompiled.instr = "MULS";
				break;
			case 0xE:
				decompiled.instr = "BICS";
				break;
			case 0xF:
				decompiled.instr = "MVNS";
				break;
			default:
				printf("Unknown ALU opcode: %.01X\n", opcode);
				break;
			}
		}
		else if ((instr >> 10) == 0x11) {
			// Hi register ops / BX
			byte op = (instr >> 8) & 0x3;
			decompiled.instr = "Somewhere above BX";
			if (op == 0x0) {

			}
			else if (op == 0x1) {

			}
			else if (op == 0x2) {

			}
			else if (op == 0x3) {
				state->pc = state->readReg((instr >> 3) & 0xF) & 0xFFFFFFFE;
				
				if (state->readReg((instr >> 3) & 0xF) & 1) state->cpsr |= 0x00000020;
				else state->cpsr &= ~0x00000020;
			}
		}
		else if ((instr >> 11) == 0x9) {
			word addr = state->pc + ((instr & 0xFF) << 2) + 2;
			if (addr & 0x2) addr ^= 0x2;
			state->writeReg((instr >> 8) & 0x7, state->read32(addr));
		}
		else if (IS_SET(instr, 9)) {
			byte Rd = (instr) & 0x7;
			byte Rb = (instr >> 3) & 0x7;
			byte Ro = (instr >> 6) & 0x7;
			//decompiled.arg0 = register_names[Rd] + ", [" + register_names[Rb] + ", " + register_names[Ro] + "]";

			byte opcode = (instr >> 10) & 0x3;
			switch (opcode) {
			case 0x0:
				decompiled.instr = "STRH";
				break;
			case 0x1:
				decompiled.instr = "LDSB";
				break;
			case 0x2:
				decompiled.instr = "LDRH";
				break;
			case 0x3:
				decompiled.instr = "LDSH";
				break;
			}

		}
		else {
			decompiled.instr = IS_SET(instr, 11) ? "LDR" : "STR";
			if (IS_SET(instr, 10)) {
				if (IS_SET(instr, 11)) state->writeReg((instr & 0x7), state->read32(state->readReg((instr >> 3) & 0x7) + state->readReg((instr >> 6) & 0x7)));
				else state->write32(state->readReg((instr >> 3) & 0x7) + state->readReg((instr >> 6) & 0x7), state->readReg((instr & 0x7)));
			}
			else {
				if (IS_SET(instr, 11)) state->writeReg((instr & 0x7), state->read8(state->readReg((instr >> 3) & 0x7) + state->readReg((instr >> 6) & 0x7)));
				else state->write8(state->readReg((instr >> 3) & 0x7) + state->readReg((instr >> 6) & 0x7), state->readReg((instr & 0x7)));
			}
		}
	}
	else if ((instr >> 13) == 0x3) {
		decompiled.instr = IS_SET(instr, 11) ? "LDR" : "STR";
		if (IS_SET(instr, 12)) decompiled.instr += "B";
		word offset = (instr >> 6) & 0x1F;
		if (!IS_SET(instr, 12)) offset <<= 2;
		//decompiled.arg0 = register_names[(instr & 0x7)] + ", [" + register_names[(instr >> 3) & 0x7] + ", #" + to_hex(offset, 0) + "]";
	}
	else if ((instr >> 13) == 0x4) {
		if (IS_SET(instr, 12)) {
			decompiled.instr = IS_SET(instr, 11) ? "LDR" : "STR";
			//decompiled.arg0 = register_names[((instr >> 8) & 0x7)] + ", [ SP, #" + to_hex((instr & 0xFF) << 2, 0) + "]";
		}
		else {
			byte Rd = (instr) & 0x7;
			byte Rb = (instr >> 3) & 0x7;
			byte offset = (instr >> 6) & 0x1F;
			decompiled.instr = IS_SET(instr, 11) ? "LDRH" : "STRH";
			//decompiled.arg0 = register_names[Rd] + ", [" + register_names[Rb] + ", #" + to_hex(offset << 1, 0) + "]";
		}
	}
	else if ((instr >> 13) == 0x5) {
		if (IS_SET(instr, 12)) {
			if (IS_SET(instr, 10)) {
				decompiled.instr = IS_SET(instr, 11) ? "POP" : "PUSH";
				decompiled.arg0 = "(";
				bool started = false;
				for (int i = 0; i < 8; i++) {
					if (IS_SET(instr, i)) {
						//decompiled.arg0 += (started ? ", " : "") + register_names[i];
						started = true;
					}
				}
				if (IS_SET(instr, 8)) decompiled.arg0 += std::string(started ? ", " : "") + (IS_SET(instr, 11) ? "LR" : "PC");
				decompiled.arg0 += ")";
			}
			else {
				decompiled.instr = IS_SET(instr, 7) ? "SUB" : "ADD";
				//decompiled.arg0 = "SP, SP, #" + to_hex((instr & 0x7F) << 2, 0);
			}
		}
		else {
			// Load address
			decompiled.instr = "Load address...";
		}
	}
	else if ((instr >> 13) == 0x6) {
		if (IS_SET(instr, 12)) {
			byte cond = (instr >> 8) & 0xF;
			if (cond == 0xF) {
				// SWI
				decompiled.instr = "SWI";
			}
			else {
				word addr = (instr & 0xFF) << 1;
				if (IS_SET(addr, 8)) addr |= 0xFFFFFF00;
				addr += state->pc + 2;

				switch (cond) {
				case 0x0: if (state->cpsr & FLAG_ZERO) break; else return;
				case 0x1: if (state->cpsr & FLAG_ZERO) return; else break;
				case 0x2: if (state->cpsr & FLAG_CARRY) break; else return;
				case 0x3: if (state->cpsr & FLAG_CARRY) return; else break;
				case 0x4: if (state->cpsr & FLAG_NEGATIVE) break; else return;
				case 0x5: if (state->cpsr & FLAG_NEGATIVE) return; else break;
				case 0x6: if (state->cpsr & FLAG_OVERFLOW) break; else return;
				case 0x7: if (state->cpsr & FLAG_OVERFLOW) return; else break;
				case 0x8: if ((state->cpsr & FLAG_CARRY) && !(state->cpsr & FLAG_CARRY)) break; else return;
				case 0x9: if (!(state->cpsr & FLAG_CARRY) && (state->cpsr & FLAG_CARRY)) break; else return;
				case 0xA: if (((state->cpsr & FLAG_NEGATIVE) >> 3) == (state->cpsr & FLAG_OVERFLOW)) break; else return;
				case 0xB: if (((state->cpsr & FLAG_NEGATIVE) >> 3) == (state->cpsr & FLAG_OVERFLOW)) return; else break;
				case 0xC: if (((state->cpsr & FLAG_NEGATIVE) >> 3) == (state->cpsr & FLAG_OVERFLOW) && !(state->cpsr & FLAG_ZERO)) break; else return;
				case 0xD: if (((state->cpsr & FLAG_NEGATIVE) >> 3) == (state->cpsr & FLAG_OVERFLOW) && !(state->cpsr & FLAG_ZERO)) return; else break;
				case 0xE: break; // Always
				case 0xF: printf("This condition should not happen...\n"); return;
				}

				state->pc = addr;
			}
		}
		else; // Multiple LOAD/STORE
	}
	else if ((instr >> 13) == 0x7) {
		if (IS_SET(instr, 12)) {
			decompiled.instr = "BL";

			state->pc += 2;
			hword p2 = state->read16(state->pc); // Read second instruction

			word offset = 0;
			offset += (instr & 0x7FF) << (IS_SET(instr, 11) ? 1 : 12);
			offset += (p2 & 0x7FF) << (IS_SET(p2, 11) ? 1 : 12);
			if (IS_SET(offset, 22)) offset |= 0xFF800000;
			offset += state->pc + 2;

			//decompiled.arg0 = to_hex(offset);
		}
		else {
			decompiled.instr = "B";
			word offset = 0;
			offset += (instr & 0x7FF) << 1;
			if (IS_SET(offset, 11)) offset |= 0xFFFFF000;
			offset += state->pc + 4;
			//decompiled.arg0 = to_hex(offset);
		}
	}
}

void ARM::Debugger::executeNextInstruction() {
	printf("[DEBUGGER][%.08X]", state->pc);
	if (IS_SET(state->cpsr, 5)) {
		hword instr = state->read16(state->pc);
		state->pc += 2;
		executeNextTHUMB(instr);
	}
	else {
		word instr = state->read32(state->pc);
		state->pc += 4;
		executeNextARM(instr);
	}
}

void ARM::Debugger::printRegisters() {
	for (int i = 0; i <= 15; i++) {
		printf("[DEBUGGER][INFO] R%.02d: %.08X\n", i, state->readReg(i));
	}
	printf("[DEBUGGER][INFO] CPSR: %.08X\n--> %s%s%s%s, %s\n", state->cpsr, state->cpsr & FLAG_NEGATIVE ? "N" : ".", state->cpsr & FLAG_ZERO ? "Z" : ".", state->cpsr & FLAG_CARRY ? "C" : ".", state->cpsr & FLAG_OVERFLOW ? "V" : ".", state->cpsr & 0x20 ? "THUMB" : "ARM");
}