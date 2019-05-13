#include <decompiler/Decompiler.h>
#include <debugger/Debugger.h>
#include <util/FileReader.h>

using namespace ARM;

static void debug(byte* bios) {
	Debugger* debugger = new Debugger(bios);
	Decompiler* decompiler = new Decompiler(debugger->state);

	while (true) {
		debugger->printRegisters();
		printf("[TEST][%.08X] Next Instruction: ", debugger->state->pc);
		DecompiledInstr next = decompiler->decompileInstruction();
		next.print();

		std::string userInput;
		std::cin >> userInput;

		if (userInput == "n") {
			int num = 1;

			if(std::cin.peek() != 0xA) std::cin >> num;

			for (int i = 0; i < num; i++) {
				DecompiledInstr a = decompiler->decompileInstruction();
				debugger->executeNextInstruction();
				a.print();
			}
		}
		else if (userInput == "u") {
			int num = decompiler->state->pc;

			if (std::cin.peek() != 0xA) std::cin >> std::hex >> num;

			while (num != decompiler->state->pc) {
				DecompiledInstr a = decompiler->decompileInstruction();
				debugger->executeNextInstruction();
				a.print();
			}
		}
		else if (userInput == "quit") {
			break;
		}

	}

	delete debugger;
}

int main(int argc, char **args) {
	byte* bios = readFile("D:\\Dropbox\\GBA-Emu-2\\GBA-Emu-2\\res\\roms\\bios.gba");
	// byte* rom = readFile("D:\\Dropbox\\GBA-Emu-2\\GBA-Emu-2\\res\\roms\\bios.gba");

	debug(bios);

	free(bios);
}