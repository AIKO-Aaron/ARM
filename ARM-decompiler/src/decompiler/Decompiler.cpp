#include "Decompiler.h"

template <typename T>
std::string to_hex(T num, int w = 2 * sizeof(T)) {
	std::stringstream stream;
	stream << "0x" << std::setfill('0') << std::setw(w) << std::hex << std::uppercase << num;
	return stream.str();
}

static std::string* register_names = new std::string[16] {
	"R0",
	"R1",
	"R2",
	"R3",
	"R4",
	"R5",
	"R6",
	"R7",
	"R8",
	"R9",
	"R10",
	"R11",
	"R12",
	"SP",
	"LR",
	"PC",
};

static std::string* condition_names = new std::string[16]{
	"EQ",
	"NE",
	"CS",
	"CC",
	"MI",
	"PL",
	"VS",
	"VC",
	"HI",
	"LS",
	"GE",
	"LT",
	"GT",
	"LE",
	"", // ALways
	"NV",
};

ARM::DecompiledInstr ARM::Decompiler::decompileARM(word instr) {
	ARM::DecompiledInstr decompiled;

	byte condition = instr >> 28;
	decompiled.cond = condition_names[condition];

	if (IS_SET(instr, 27)) {
		if (IS_SET(instr, 26)) {

		}
		else {
			if (IS_SET(instr, 25)) {
				bool link = IS_SET(instr, 24);
				word offset = (instr & 0x00FFFFFF) << 2;
				if (IS_SET(offset, 25)) offset |= 0xFC;
				offset += state->pc + 8;

				decompiled.instr = "B";
				if (link) decompiled.instr += "L";

				decompiled.arg0 = to_hex(offset);
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
				decompiled.arg0 = register_names[Rd] + ", [" + register_names[Rn];

				if (imm || offset != 0) { // LDR/STR Rd, [Rn, Rm SHIFT/IMM]
					if (Rn == 0xF) {
						decompiled.arg0 += "] ; =" + to_hex(state->read32(state->pc + offset + 8));
					}
					else {
						if (imm) decompiled.arg0 += ", " + register_names[offset & 0xF] + "...]";
						else decompiled.arg0 += ", #" + to_hex(offset, 0) + "]";
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
				decompiled.arg0 += ", " + register_names[instr & 0xF];
			}
			else if (((instr >> 12) & 0x3FF) == 0x28F && ((instr >> 23) & 0x1B) == 0x02) {
				printf("MSR Flag bits only\n");
			}
			else if ((instr & 0x0FFFFFF0) == 0x012FFF10) {
				decompiled.instr = "BX";
				decompiled.arg0 = register_names[instr & 0xF];
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

				decompiled.arg0 = register_names[Rd] + ", " + register_names[Rn];

				switch (opcode) {
				case 0x4:
					if (Rn == 0xF) {
						decompiled.instr = "ADR";
						decompiled.arg0 = register_names[Rd] + ", #" + to_hex(state->pc + 8 + op2, 0);
						decompiled.arg0 += std::string(" ; ") + ((op2 & 1) ? "(To THUMB)" : "(To ARM)");
					}
					else {
						decompiled.instr = "ADD";
						decompiled.arg0 = register_names[Rd] + "," + register_names[Rn] + ", #" + to_hex(op2, 0);
					}
					break;
				case 0x9:
					decompiled.instr = "TEQ";
					decompiled.arg0 = register_names[Rn] + ", #" + to_hex(op2, 0);
					break;
				case 0x0A:
					decompiled.instr = "CMP";
					decompiled.arg0 = register_names[Rn] + ", #" + to_hex(op2, 0);
					break;
				case 0xD:
					decompiled.instr = "MOV";
					decompiled.arg0 = register_names[Rd] + ", #" + to_hex(op2, 0);
					break;
				default:
					printf("Data processing: %.02X\n", opcode);
					break;
				}
			}
		}
	}

	state->pc += 4;

	return decompiled;
}

ARM::DecompiledInstr ARM::Decompiler::decompileTHUMB(halfword instr) {
	DecompiledInstr decompiled;
	
	// printf("%.04X\n", instr);

	if ((instr >> 13) == 0x0) {
		if ((instr >> 11) == 0x3) {
			decompiled.instr = IS_SET(instr, 9) ? "SUBS" : "ADDS";
			decompiled.arg0 = register_names[instr & 0x7] + ", " + register_names[(instr >> 3) & 0x7] + ", ";
			if (IS_SET(instr, 10)) decompiled.arg0 += "#" + to_hex((instr >> 6) & 0x7, 0);
			else decompiled.arg0 += register_names[(instr >> 6) & 0x7];
		}
		else {
			byte Rd = (instr) & 0x7;
			byte Rs = (instr >> 3) & 0x7;
			byte offset = (instr >> 6) & 0x1F;
			decompiled.arg0 = register_names[Rd] + ", " + register_names[Rs] + ", #" + to_hex((word)offset, 0);

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
	} else if ((instr >> 13) == 0x1) {
		byte opcode = (instr >> 11) & 3;
		byte Rd = (instr >> 8) & 0x7;
		byte offset = (instr & 0xFF);
		decompiled.arg0 = register_names[Rd] + ", #" + to_hex((word) offset, 0);

		switch (opcode) {
		case 0x0:
			decompiled.instr = "MOVS";
			break;
		case 0x1:
			decompiled.instr = "CMP";
			break;
		case 0x2:
			decompiled.instr = "ADDS";
			break;
		case 0x3:
			decompiled.instr = "SUBS";
			break;
		}
	} else if ((instr >> 13) == 0x2) {
		if ((instr >> 10) == 0x10) {
			byte opcode = (instr >> 6) & 0xF;

			decompiled.arg0 = register_names[instr & 0x7] + ", " + register_names[(instr >> 3) & 0x7];
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
				decompiled.instr = "BX";
				decompiled.arg0 = register_names[(instr >> 3) & 0xF];
			}
		}
		else if ((instr >> 11) == 0x9) {
			word addr = state->pc + ((instr & 0xFF) << 2) + 4;
			if (addr & 0x2) addr ^= 0x2;
			decompiled.instr = "LDR";
			decompiled.arg0 = register_names[(instr >> 8) & 0x7];
			decompiled.arg0 += ", =" + to_hex(state->read32(addr));
		}
		else if (IS_SET(instr, 9)) {
			byte Rd = (instr) & 0x7;
			byte Rb = (instr >> 3) & 0x7;
			byte Ro = (instr >> 6) & 0x7;
			decompiled.arg0 = register_names[Rd] + ", [" + register_names[Rb] + ", " + register_names[Ro] + "]";

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
			if (IS_SET(instr, 10)) decompiled.instr += "B";

			decompiled.arg0 = register_names[(instr & 0x7)] + ", [" + register_names[(instr >> 3) & 0x7] + ", " + register_names[(instr >> 6) & 0xF] + "]";
		}
	} else if ((instr >> 13) == 0x3) {
		decompiled.instr = IS_SET(instr, 11) ? "LDR" : "STR";
		if (IS_SET(instr, 12)) decompiled.instr += "B";
		word offset = (instr >> 6) & 0x1F;
		if (!IS_SET(instr, 12)) offset <<= 2;
		decompiled.arg0 = register_names[(instr & 0x7)] + ", [" + register_names[(instr >> 3) & 0x7] + ", #" + to_hex(offset, 0) + "]";
	} else if ((instr >> 13) == 0x4) {
		if (IS_SET(instr, 12)) {
			decompiled.instr = IS_SET(instr, 11) ? "LDR" : "STR";
			decompiled.arg0 = register_names[((instr >> 8) & 0x7)] + ", [ SP, #" + to_hex((instr & 0xFF) << 2, 0) + "]";
		}
		else {
			byte Rd = (instr) & 0x7;
			byte Rb = (instr >> 3) & 0x7;
			byte offset = (instr >> 6) & 0x1F;
			decompiled.instr = IS_SET(instr, 11) ? "LDRH" : "STRH";
			decompiled.arg0 = register_names[Rd] + ", [" + register_names[Rb] + ", #" + to_hex(offset << 1, 0) + "]";
		}
	} else if ((instr >> 13) == 0x5) {
		if (IS_SET(instr, 12)) {
			if (IS_SET(instr, 10)) {
				decompiled.instr = IS_SET(instr, 11) ? "POP" : "PUSH";
				decompiled.arg0 = "(";
				bool started = false;
				for (int i = 0; i < 8; i++) {
					if (IS_SET(instr, i)) {
						decompiled.arg0 += (started ? ", " : "") + register_names[i];
						started = true;
					}
				}
				if (IS_SET(instr, 8)) decompiled.arg0 += std::string(started ? ", " : "") + (IS_SET(instr, 11) ? "LR" : "PC");
				decompiled.arg0 += ")";
			}
			else {
				decompiled.instr = IS_SET(instr, 7) ? "SUB" : "ADD";
				decompiled.arg0 = "SP, SP, #" + to_hex((instr & 0x7F) << 2, 0);
			}
		}
		else {
			// Load address
			decompiled.instr = "Load address...";
		}
	} else if ((instr >> 13) == 0x6) {
		if (IS_SET(instr, 12)) {
			byte cond = (instr >> 8) & 0xF;
			if (cond == 0xF) {
				// SWI
				decompiled.instr = "SWI";
			}
			else {
				decompiled.instr = "B" + condition_names[cond];
				word addr = (instr & 0xFF) << 1;
				if (IS_SET(addr, 8)) addr |= 0xFFFFFF00;
				addr += state->pc + 4;
				decompiled.arg0 = "#" + to_hex(addr, 0);
			}
		}
		else; // Multiple LOAD/STORE
	} else if ((instr >> 13) == 0x7) {
		if (IS_SET(instr, 12)) {
			decompiled.instr = "BL";

			state->pc += 2;
			halfword p2 = state->read16(state->pc); // Read second instruction

			word offset = 0;
			offset += (instr & 0x7FF) << (IS_SET(instr, 11) ? 1 : 12);
			offset += (p2 & 0x7FF) << (IS_SET(p2, 11) ? 1 : 12);
			if (IS_SET(offset, 22)) offset |= 0xFF800000;
			offset += state->pc + 2;

			decompiled.arg0 = to_hex(offset);
		}
		else {
			decompiled.instr = "B";
			word offset = 0;
			offset += (instr & 0x7FF) << 1;
			if (IS_SET(offset, 11)) offset |= 0xFFFFF000;
			offset += state->pc + 4;
			decompiled.arg0 = to_hex(offset);
		}
	} 

	state->pc += 2;

	return decompiled;
}