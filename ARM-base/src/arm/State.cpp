#include "State.h"

word ARM::State::readReg(byte num) {
	switch (num) {
	case 0x0: return r0;
	case 0x1: return r1;
	case 0x2: return r2;
	case 0x3: return r3;
	case 0x4: return r4;
	case 0x5: return r5;
	case 0x6: return r6;
	case 0x7: return r7;
	case 0x8: return r8;
	case 0x9: return r9;
	case 0xA: return r10;
	case 0xB: return r11;
	case 0xC: return r12;
	case 0xD: return sp;
	case 0xE: return lr;
	case 0xF: return pc;
	default:
		printf("[STATE][ERROR] No such register found: 0x%.02X\n", num);
		exit(1);
	}
}

void ARM::State::writeReg(byte num, word val) {
	switch (num) {
	case 0x0: r0 = val; break;
	case 0x1: r1 = val; break;
	case 0x2: r2 = val; break;
	case 0x3: r3 = val; break;
	case 0x4: r4 = val; break;
	case 0x5: r5 = val; break;
	case 0x6: r6 = val; break;
	case 0x7: r7 = val; break;
	case 0x8: r8 = val; break;
	case 0x9: r9 = val; break;
	case 0xA: r10 = val; break;
	case 0xB: r11 = val; break;
	case 0xC: r12 = val; break;
	case 0xD: sp = val; break;
	case 0xE: lr = val; break;
	case 0xF: pc = val; break;
	default:
		printf("[STATE][ERROR] No such register found: 0x%.02X\n", num);
		exit(1);
	}
}

void ARM::State::setMode(byte mode) {
	byte currentMode = cpsr & 0x1F;

	switch (currentMode) {
	case MODE_USER:
	case MODE_SYSTEM:
		lr_reg = lr;
		sp_reg = sp;
		r12_reg = r12;
		r11_reg = r11;
		r10_reg = r10;
		r9_reg = r9;
		r8_reg = r8;
		break;
	case MODE_FIQ:
		lr_fiq = lr;
		sp_fiq = sp;
		r12_fiq = r12;
		r11_fiq = r11;
		r10_fiq = r10;
		r9_fiq = r9;
		r8_fiq = r8;
		break;
	case MODE_IRQ:
		lr_irq = lr;
		break;
	case MODE_SUPERVISOR:
		lr_svc = lr;
		break;
	case MODE_ABORT:
		lr_abt = lr;
		break;
	case MODE_UNDEFINED:
		lr_und = lr;
		break;
	default:
		printf("[STATE][ERROR] What mode are we running in? %d\n", currentMode);
		exit(1);
	}

	switch (mode) {
	case MODE_USER:
	case MODE_SYSTEM:
		lr = lr_reg;
		sp = sp_reg;
		r12 = r12_reg;
		r11 = r11_reg;
		r10 = r10_reg;
		r9 = r9_reg;
		r8 = r8_reg;
		break;
	case MODE_FIQ:
		lr = lr_fiq;
		sp = sp_fiq;
		r12 = r12_fiq;
		r11 = r11_fiq;
		r10 = r10_fiq;
		r9 = r9_fiq;
		r8 = r8_fiq;
		break;
	case MODE_IRQ:
		lr = lr_irq;
		break;
	case MODE_SUPERVISOR:
		lr = lr_svc;
		break;
	case MODE_ABORT:
		lr = lr_abt;
		break;
	case MODE_UNDEFINED:
		lr = lr_und;
		break;
	default:
		printf("[STATE][ERROR] What mode will we be running in? %d\n", mode);
		exit(1);
	}

	cpsr = (cpsr & 0xFFFFFFE0) | mode;
}