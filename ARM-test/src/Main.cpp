#include <decompiler/Decompiler.h>
#include <debugger/Debugger.h>
#include <util/FileReader.h>

using namespace ARM;

static void testDebugger(byte* bios) {
	Debugger* debugger = new Debugger(bios);
	Decompiler* decompiler = new Decompiler(debugger->state);

	for (int i = 0; i < 3; i++) {
		DecompiledInstr a = decompiler->decompileInstruction();
		debugger->executeNextInstruction();
		a.print();
	}
	delete debugger;
}

int main(int argc, char **args) {
	byte* bios = readFile("D:\\Dropbox\\GBA-Emu-2\\GBA-Emu-2\\res\\roms\\bios.gba");

	testDebugger(bios);

	free(bios);
}