#pragma once

#include <string>
#include "arm/State.h"

namespace ARM {
	
	class Debugger {
	private:
		void executeNextARM(word instr);
		void executeNextTHUMB(halfword instr);
	public:
		State* state;
		Debugger(byte* mem) : state(new State(mem)) {}

		void executeNextInstruction();
	};

}