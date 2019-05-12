#include "Debugger.h"

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
				state->pc += 4 + offset; // 4 added later
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

				decompiled.instr = load ? "LDR" : "STR";
				if (isByte) decompiled.instr += "B";
				//decompiled.arg0 = register_names[Rd] + ", [" + register_names[Rn];

				if (imm || offset != 0) { // LDR/STR Rd, [Rn, Rm SHIFT/IMM]
					if (Rn == 0xF) {
						//decompiled.arg0 += "] ; =" + to_hex(state->read32(state->pc + offset + 8));
					}
					else {
						//if (imm) decompiled.arg0 += ", " + register_names[offset & 0xF] + "...]";
						//else decompiled.arg0 += ", #" + to_hex(offset, 0) + "]";
					}
				}
				else decompiled.arg0 += "]";
			}
		}
		else {
			if (((instr >> 4) & 0x9) == 0x9 && !IS_SET(instr, 25)) {

			}
			else if (((instr >> 16) & 0x3F) == 0x0F && ((instr >> 23) & 0x1F) == 0x02 && (instr & 0xFFF) == 0) {
				printf("MRS\n");
			}
			else if (((instr >> 12) & 0x3FF) == 0x29F && ((instr >> 23) & 0x1F) == 0x02 && (instr & 0xFF0) == 0) {
				decompiled.instr = "MSR";
				decompiled.arg0 = IS_SET(instr, 22) ? "SPSR_" : "CPSR";
				//decompiled.arg0 += ", " + register_names[instr & 0xF];
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
					if (Rn == 0xF) {
						decompiled.instr = "ADR";
						//decompiled.arg0 = register_names[Rd] + ", #" + to_hex(state->pc + 8 + op2, 0);
						decompiled.arg0 += std::string(" ; ") + ((op2 & 1) ? "(To THUMB)" : "(To ARM)");
					}
					else {
						decompiled.instr = "ADD";
						//decompiled.arg0 = register_names[Rd] + "," + register_names[Rn] + ", #" + to_hex(op2, 0);
					}
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
		executeNextTHUMB(state->read16(state->pc));
		state->pc += 2;
	}
	else {
		executeNextARM(state->read32(state->pc));
		state->pc += 4;
	}
}
