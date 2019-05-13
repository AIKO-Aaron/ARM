#pragma once

#include <string>
#include "arm/State.h"

namespace ARM {
	
	class Debugger {
	private:
		void setFlags(dword b, dword c, bool addone);


		void executeNextARM(word instr);
		void executeNextTHUMB(hword instr);
	public:
		State* state;
		Debugger(byte* mem) : state(new State(mem)) {}

		void executeNextInstruction();
		void printRegisters();
	};

}