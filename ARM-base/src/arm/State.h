#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "Memory.h"

#define IS_SET(x,a) ((((x)>>(a)) & 1) == 1)

constexpr byte MODE_USER		= 0b10000;
constexpr byte MODE_FIQ			= 0b10001;
constexpr byte MODE_IRQ			= 0b10010;
constexpr byte MODE_SUPERVISOR	= 0b10011;
constexpr byte MODE_ABORT		= 0b10111;
constexpr byte MODE_UNDEFINED	= 0b11011;
constexpr byte MODE_SYSTEM		= 0b11111;

constexpr word FLAG_NEGATIVE	= 1 << 31;
constexpr word FLAG_ZERO		= 1 << 30;
constexpr word FLAG_CARRY		= 1 << 29;
constexpr word FLAG_OVERFLOW	= 1 << 28;

constexpr word FLAG_IRQ_DISABLE	= 1 << 7;
constexpr word FLAG_FIQ_DISABLE	= 1 << 6;

#define SET_FLAG(flag) (state->cpsr |= (flag))
#define CLEAR_FLAG(flag) (state->cpsr &= ~(flag))
#define IS_FLAG_SET(flag) (state->cpsr & (flag)

namespace ARM {

	class State {
	private:
		// Banked registers; only visible when selected mode is equal to the bank
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

		// Visible registers
		word cpsr = MODE_SUPERVISOR;
		word spsr = 0;
		word pc = 0;
		word lr = 0;
		word sp = 0;
		word r12 = 0, r11 = 0, r10 = 0, r9 = 0, r8 = 0, r7 = 0, r6 = 0, r5 = 0, r4 = 0, r3 = 0, r2 = 0, r1 = 0, r0 = 0;

		inline byte read8(word addr) { return mem->r8(addr); }
		inline hword read16(word addr) { return mem->r16(addr); }
		inline word read32(word addr) { return mem->r32(addr); }

		inline void write8(word addr, byte data) { return mem->w8(addr, data); }
		inline void write16(word addr, hword data) { return mem->w16(addr, data); }
		inline void write32(word addr, word data) { return mem->w32(addr, data); }

		word readReg(byte num);
		void writeReg(byte num, word val);
		void setMode(byte mode);
	};

}