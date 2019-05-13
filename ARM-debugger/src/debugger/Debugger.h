#pragma once

#include <string>
#include "arm/State.h"

namespace ARM {
	
	class Debugger {
	private:
		void setFlags(doubleword a, word b, word c);

		void executeNextARM(word instr);
		void executeNextTHUMB(halfword instr);
	public:
		State* state;
		Debugger(byte* mem) : state(new State(mem)) {}

		void executeNextInstruction();
	};

}