#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdio.h>

#include "arm/State.h"

namespace ARM {
	
	struct DecompiledInstr {
		std::string instr = "";
		std::string cond = "";
		std::string arg0 = "";


		inline void print() {
			printf("%5s%2s %s\n", instr.c_str(), cond.c_str(), arg0.c_str());
		}
	};

	class Decompiler {
	private:
	public:
		Decompiler(byte* m) : state(new State(m)) {}
		Decompiler(State* s) : state(s) {}

		State *state;

		DecompiledInstr decompileInstruction();
		DecompiledInstr decompileARM(word instr);
		DecompiledInstr decompileTHUMB(halfword instr);
	};

}
