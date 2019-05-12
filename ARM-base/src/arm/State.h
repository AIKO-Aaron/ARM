#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "Memory.h"

#define IS_SET(x,a) ((((x)>>(a)) & 1) == 1)

constexpr byte MODE_USER = 0b10000;
constexpr byte MODE_FIQ = 0b10001;
constexpr byte MODE_IRQ = 0b10010;
constexpr byte MODE_SUPERVISOR = 0b10011;
constexpr byte MODE_ABORT = 0b10111;
constexpr byte MODE_UNDEFINED = 0b11011;
constexpr byte MODE_SYSTEM = 0b11111;

namespace ARM {

	class State {
	private:
		word lr_reg = 0, lr_fiq = 0, lr_svc = 0, lr_abt = 0, lr_irq = 0, lr_und = 0;
		word sp_reg = 0, sp_fiq = 0, sp_svc = 0, sp_abt = 0, sp_irq = 0, sp_und = 0;
		word spsr_fiq = 0, spsr_svc = 0, spsr_abt = 0, spsr_irq = 0, spsr_und = 0;

		word r12_reg = 0, r12_fiq = 0;
		word r11_reg = 0, r11_fiq = 0;
		word r10_reg = 0, r10_fiq = 0;
		word r9_reg = 0, r9_fiq = 0;
		word r8_reg = 0, r8_fiq = 0;

		Memory* mem;

	public:
		State(byte* m) : mem(new Memory(m)) {}

		word cpsr = MODE_SUPERVISOR;
		word spsr = 0;

		word pc = 0;
		word lr = 0;
		word sp = 0;

		word r12 = 0, r11 = 0, r10 = 0, r9 = 0, r8 = 0, r7 = 0, r6 = 0, r5 = 0, r4 = 0, r3 = 0, r2 = 0, r1 = 0, r0 = 0;

		inline byte read8(word addr) { return mem->r8(addr); }
		inline halfword read16(word addr) { return mem->r16(addr); }
		inline word read32(word addr) { return mem->r32(addr); }

		word readReg(byte num);
		void writeReg(byte num, word val);
		void setMode(byte mode);
	};

}