#pragma once

#include <stdint.h>

typedef uint64_t doubleword;
typedef uint32_t word;
typedef uint16_t halfword;
typedef uint8_t byte;

namespace ARM {
	class Memory {
	private:
		byte* memory;

	public:
		Memory(byte* m) : memory(m) {}

		inline byte r8(word addr) { return memory[addr]; }
		inline halfword r16(word addr) { return *(halfword*)(memory + addr); }
		inline word r32(word addr) { return *(word*)(memory + addr); }
	};
}