#include <decompiler/Decompiler.h>
#include <debugger/Debugger.h>

#include "FileReader.h"

using namespace ARM;

int main(int argc, char **args) {
	byte* bios = readFile("D:\\Dropbox\\GBA-Emu-2\\GBA-Emu-2\\res\\roms\\bios.gba");
	word* bios_words = (word*)bios;

	word b = bios_words[0x1A];
	printf("%.08X\n", b);

	Decompiler *d = new Decompiler(bios);
	DecompiledInstr a;

	a = d->decompileARM(bios_words[0]).print();
	d->state->pc = 0x68;
	a = d->decompileARM(bios_words[0x1A]).print();
	a = d->decompileARM(bios_words[0x1B]).print();
	a = d->decompileARM(bios_words[0x1C]).print();
	a = d->decompileARM(bios_words[0x1D]).print();
	a = d->decompileARM(bios_words[0x1E]).print();
	a = d->decompileARM(bios_words[0x23]).print();
	a = d->decompileARM(bios_words[0x24]).print();
	a = d->decompileARM(bios_words[0x25]).print();
	a = d->decompileARM(bios_words[0x26]).print();

	d->state->pc = 0x9C;
	a = d->decompileARM(bios_words[0x27]).print();

	d->state->pc = 0xE0;
	for (int i = 0x38; i <= 0x46; i++) {
		a = d->decompileARM(bios_words[i]).print();
	}

	d->state->pc = 0x11C; // +1 --> thumb
	for (int i = 0; i < 6; i++) {
		a = d->decompileTHUMB(d->state->read16(d->state->pc)).print();
	}

	d->state->pc = 0xA0;
	for (int i = 0; i < 5; i++) a = d->decompileARM(d->state->read32(d->state->pc)).print();

	d->state->pc = 0x1928;
	for(int i = 0; i < 120; i++) a = d->decompileTHUMB(d->state->read16(d->state->pc)).print();
}