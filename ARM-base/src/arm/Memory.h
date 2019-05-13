#pragma once

#include <stdint.h>
#include <string>

typedef uint64_t dword;
typedef uint32_t word;
typedef uint16_t hword;
typedef uint8_t byte;

namespace ARM {
	class Memory {
	private:
		byte* memory;

		byte* wram = (byte*) malloc(0x40000);
		byte* iram = (byte*) malloc(0x08000);
		byte* io = (byte*) malloc(0x3FF);

	public:
		Memory(byte* m) : memory(m) {
			memset(wram, 0, 0x40000);
			memset(iram, 0, 0x08000);
			memset(io, 0, 0x3FF);
		}

		inline byte r8(word addr) { 
			switch (addr >> 24) {
			case 0x0:
				return memory[addr & 0xFFFFFF];
			case 0x2:
				return wram[addr & 0xFFFFFF];
			case 0x3:
				return iram[addr & 0xFFFFFF];
			case 0x4:
				return io[addr & 0xFFFFFF];
			default:
				printf("[ERROR] No such memory address: %.08X\n", addr);
				break;
			} 
			return memory[addr]; 
		}

		inline hword r16(word addr) { 
			switch (addr >> 24) {
			case 0x0:
				return *(hword*)(memory + (addr & 0xFFFFFF));
			case 0x2:
				return *(hword*)(wram + (addr & 0xFFFFFF));
			case 0x3:
				return *(hword*)(iram + (addr & 0xFFFFFF));
			case 0x4:
				return *(hword*)(io + (addr & 0xFFFFFF));
			default:
				printf("[ERROR] No such memory address: %.08X\n", addr);
				break;
			}

			return *(hword*)(memory + addr);
		}
		inline word r32(word addr) { 
			switch (addr >> 24) {
			case 0x0:
				return *(word*)(memory + (addr & 0xFFFFFF));
			case 0x2:
				return *(word*)(wram + (addr & 0xFFFFFF));
			case 0x3:
				return *(word*)(iram + (addr & 0xFFFFFF));
			case 0x4:
				return *(word*)(io + (addr & 0xFFFFFF));
			default:
				printf("[ERROR] No such memory address: %.08X\n", addr);
				break;
			}
			return *(word*)(memory + addr); 
		}

		inline void w8(word addr, byte data) {
			switch (addr >> 24) {
			case 0x0:
				memory[addr & 0x00FFFFFF] = data;
				break;
			case 0x2:
				wram[addr & 0xFFFFFF];
				break;
			case 0x3:
				iram[addr & 0xFFFFFF];
				break;
			case 0x4:
				io[addr & 0xFFFFFF];
				break;
			default:
				printf("[ERROR] No such memory address: %.08X\n", addr);
				break;
			}
		}

		inline void w16(word addr, hword data) { 
			switch (addr >> 24) {
			case 0x0:
				*(hword*)(memory + (addr & 0x00FFFFFF)) = data;
				break;
			case 0x2:
				*(hword*)(wram + (addr & 0x00FFFFFF)) = data;
				break;
			case 0x3:
				*(hword*)(iram + (addr & 0x00FFFFFF)) = data;
				break;
			case 0x4:
				*(hword*)(io + (addr & 0x00FFFFFF)) = data;
				break;
			default:
				printf("[ERROR] No such memory address: %.08X\n", addr);
				break;
			}
		}
		inline void w32(word addr, word data) { 
			switch (addr >> 24) {
			case 0x0:
				*(word*)(memory + (addr & 0x00FFFFFF)) = data;
				break;
			case 0x2:
				*(word*)(wram + (addr & 0x00FFFFFF)) = data;
				break;
			case 0x3:
				*(word*)(iram + (addr & 0x00FFFFFF)) = data;
				break;
			case 0x4:
				*(word*)(io + (addr & 0x00FFFFFF)) = data;
				break;
			default:
				printf("[ERROR] No such memory address: %.08X\n", addr);
				break;
			}
		}
	};
}