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


		inline DecompiledInstr print() {
			printf("[DECOMPILER][INFO] %5s%2s %s\n", instr.c_str(), cond.c_str(), arg0.c_str());
			return *this;
		}
	};

	class Decompiler {
	private:
	public:
		Decompiler(byte* m) : state(new State(m)) {}

		State *state;
		DecompiledInstr decompileARM(word instr);
		DecompiledInstr decompileTHUMB(halfword instr);
	};

}
