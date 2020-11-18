#include "c64/cpu_6502.hpp"

CPU6502::~CPU6502() = default;

CPU6502::CPU6502() {
    pc = 0x0000;
    a = 0x00;
    x = 0x00;
    y = 0x00;
    stkp = 0x00;
    status = 0x00;

    addr_abs = 0x0000;
    addr_rel = 0x00;
    implied = 0x00;
    implied_has_value = false;
    cycles = 0x00;
    clock_count = 0;
}

void CPU6502::reset(CPUIO &bus) {
    uint16_t lo = bus.read(0xFFFC);
    uint16_t hi = bus.read(0xFFFC + 1);
    pc = (hi << 8) | lo;
    a = 0x00;
    x = 0x00;
    y = 0x00;
    stkp = 0xFD;
    status = 0x00 | Flags6502::U | Flags6502::I;

    addr_abs = 0x0000;
    addr_rel = 0x00;
    implied = 0x00;
    implied_has_value = false;
    cycles = 8;
}

void CPU6502::clock(CPUIO &bus) {
    if (cycles == 0) {
        opcode = bus.read(pc);
        pc++;
        implied = 0x00;
        implied_has_value = false;

        run_instruction(bus);
    }
    cycles -= 1;
    clock_count++;
}

bool CPU6502::complete() const {
    return cycles == 0;
}

uint8_t CPU6502::fetch(CPUIO &bus) {
    if (implied_has_value) {
        return implied;
    } else {
        return bus.read(addr_abs);
    }
}

uint8_t CPU6502::get_flag(Flags6502 flag) const {
    if ((status & flag) != 0) {
        return 1;
    } else {
        return 0;
    }
}

void CPU6502::set_status_flag(Flags6502 flag, bool value) {
    if (value) {
        status = status | flag;
    } else {
        status = status & ~flag;
    }
}

bool CPU6502::is_status_flag_set(Flags6502 flag) const {
    return is_flag_set(flag, status);
}

bool is_flag_set(uint8_t flag, uint8_t flags) {
    return (flags & flag) == flag;
}

void CPU6502::irq(CPUIO & bus) {
    if (!is_status_flag_set(Flags6502::I)) {
        push_program_counter_on_stack(bus);
        push_interrupt_state_on_stack(bus);
        load_program_counter_from_addr(bus, 0xFFFE);
        cycles = 7;
    }
}

void CPU6502::nmi(CPUIO & bus) {
    push_program_counter_on_stack(bus);
    push_interrupt_state_on_stack(bus);
    load_program_counter_from_addr(bus, 0xFFFA);
    cycles = 8;
}

void CPU6502::push_interrupt_state_on_stack(CPUIO & bus) {
    set_status_flag(Flags6502::B, false);
    set_status_flag(Flags6502::U, true);
    set_status_flag(Flags6502::I, true);
    push_value_on_stack(bus, status);
}

void CPU6502::push_program_counter_on_stack(CPUIO &bus) {
    push_value_on_stack(bus, (pc >> 8));
    push_value_on_stack(bus, pc);
}

void CPU6502::pop_program_counter_from_stack(CPUIO &bus) {
    auto lo = pop_value_from_stack(bus);
    auto hi = pop_value_from_stack(bus);
    pc = (hi << 8) | lo;
}

void CPU6502::push_value_on_stack(CPUIO &bus, uint8_t value) {
    bus.write(0x0100 + stkp, value);
    stkp--;
}

uint8_t CPU6502::pop_value_from_stack(CPUIO &bus) {
    stkp++;
    return bus.read(0x0100 + stkp);
}

void CPU6502::load_program_counter_from_addr(CPUIO &bus, uint16_t addr) {
    auto lo = bus.read(addr);
    auto hi = bus.read(addr + 1);
    pc = (hi << 8) | lo;
}

uint8_t CPU6502::run_instruction(CPUIO &bus) {
    switch (opcode) {
        case 0x00: { return cycles += 7 + (IMM(bus) & BRK(bus)); }
        case 0x01: { return cycles += 6 + (IZX(bus) & ORA(bus)); }
        case 0x02: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x03: { return cycles += 8 + (IZX(bus) & SLO(bus)); }
        case 0x04: { return cycles += 3 + (ZP0(bus) & NOP(bus)); }
        case 0x05: { return cycles += 3 + (ZP0(bus) & ORA(bus)); }
        case 0x06: { return cycles += 5 + (ZP0(bus) & ASL(bus)); }
        case 0x07: { return cycles += 5 + (ZP0(bus) & SLO(bus)); }
        case 0x08: { return cycles += 3 + (IMP(bus) & PHP(bus)); }
        case 0x09: { return cycles += 2 + (IMM(bus) & ORA(bus)); }
        case 0x0A: { return cycles += 2 + (IMP(bus) & ASL(bus)); }
        case 0x0B: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x0C: { return cycles += 4 + (ABS(bus) & NOP(bus)); }
        case 0x0D: { return cycles += 4 + (ABS(bus) & ORA(bus)); }
        case 0x0E: { return cycles += 6 + (ABS(bus) & ASL(bus)); }
        case 0x0F: { return cycles += 6 + (ABS(bus) & SLO(bus)); }

        case 0x10: { return cycles += 2 + (REL(bus) & BPL(bus)); }
        case 0x11: { return cycles += 5 + (IZY(bus) & ORA(bus)); }
        case 0x12: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x13: { return cycles += 8 + (IZY(bus) & SLO(bus)); }
        case 0x14: { return cycles += 4 + (ZPX(bus) & NOP(bus)); }
        case 0x15: { return cycles += 4 + (ZPX(bus) & ORA(bus)); }
        case 0x16: { return cycles += 6 + (ZPX(bus) & ASL(bus)); }
        case 0x17: { return cycles += 6 + (ZPX(bus) & SLO(bus)); }
        case 0x18: { return cycles += 2 + (IMP(bus) & CLC(bus)); }
        case 0x19: { return cycles += 4 + (ABY(bus) & ORA(bus)); }
        case 0x1A: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0x1B: { return cycles += 7 + (ABY(bus) & SLO(bus)); }
        case 0x1C: { return cycles += 4 + (ABX(bus) & NOP(bus)); }
        case 0x1D: { return cycles += 4 + (ABX(bus) & ORA(bus)); }
        case 0x1E: { return cycles += 7 + (ABX(bus) & ASL(bus)); }
        case 0x1F: { return cycles += 7 + (ABX(bus) & SLO(bus)); }

        case 0x20: { return cycles += 6 + (ABS(bus) & JSR(bus)); }
        case 0x21: { return cycles += 6 + (IZX(bus) & AND(bus)); }
        case 0x22: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x23: { return cycles += 8 + (IZX(bus) & RLA(bus)); }
        case 0x24: { return cycles += 3 + (ZP0(bus) & BIT(bus)); }
        case 0x25: { return cycles += 3 + (ZP0(bus) & AND(bus)); }
        case 0x26: { return cycles += 5 + (ZP0(bus) & ROL(bus)); }
        case 0x27: { return cycles += 5 + (ZP0(bus) & RLA(bus)); }
        case 0x28: { return cycles += 4 + (IMP(bus) & PLP(bus)); }
        case 0x29: { return cycles += 2 + (IMM(bus) & AND(bus)); }
        case 0x2A: { return cycles += 2 + (IMP(bus) & ROL(bus)); }
        case 0x2B: { return cycles += 2 + (IMM(bus) & ANC(bus)); }
        case 0x2C: { return cycles += 4 + (ABS(bus) & BIT(bus)); }
        case 0x2D: { return cycles += 4 + (ABS(bus) & AND(bus)); }
        case 0x2E: { return cycles += 6 + (ABS(bus) & ROL(bus)); }
        case 0x2F: { return cycles += 6 + (ABS(bus) & RLA(bus)); }

        case 0x30: { return cycles += 2 + (REL(bus) & BMI(bus)); }
        case 0x31: { return cycles += 5 + (IZY(bus) & AND(bus)); }
        case 0x32: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x33: { return cycles += 8 + (IZY(bus) & RLA(bus)); }
        case 0x34: { return cycles += 4 + (ZPX(bus) & NOP(bus)); }
        case 0x35: { return cycles += 4 + (ZPX(bus) & AND(bus)); }
        case 0x36: { return cycles += 6 + (ZPX(bus) & ROL(bus)); }
        case 0x37: { return cycles += 6 + (ZPX(bus) & RLA(bus)); }
        case 0x38: { return cycles += 2 + (IMP(bus) & SEC(bus)); }
        case 0x39: { return cycles += 4 + (ABY(bus) & AND(bus)); }
        case 0x3A: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0x3B: { return cycles += 7 + (ABY(bus) & RLA(bus)); }
        case 0x3C: { return cycles += 4 + (ABX(bus) & NOP(bus)); }
        case 0x3D: { return cycles += 4 + (ABX(bus) & AND(bus)); }
        case 0x3E: { return cycles += 7 + (ABX(bus) & ROL(bus)); }
        case 0x3F: { return cycles += 7 + (ABX(bus) & RLA(bus)); }

        case 0x40: { return cycles += 6 + (IMP(bus) & RTI(bus)); }
        case 0x41: { return cycles += 6 + (IZX(bus) & EOR(bus)); }
        case 0x42: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x43: { return cycles += 8 + (IZX(bus) & SRE(bus)); }
        case 0x44: { return cycles += 3 + (ZP0(bus) & NOP(bus)); }
        case 0x45: { return cycles += 3 + (ZP0(bus) & EOR(bus)); }
        case 0x46: { return cycles += 5 + (ZP0(bus) & LSR(bus)); }
        case 0x47: { return cycles += 5 + (ZP0(bus) & SRE(bus)); }
        case 0x48: { return cycles += 3 + (IMP(bus) & PHA(bus)); }
        case 0x49: { return cycles += 2 + (IMM(bus) & EOR(bus)); }
        case 0x4A: { return cycles += 2 + (IMP(bus) & LSR(bus)); }
        case 0x4B: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x4C: { return cycles += 3 + (ABS(bus) & JMP(bus)); }
        case 0x4D: { return cycles += 4 + (ABS(bus) & EOR(bus)); }
        case 0x4E: { return cycles += 6 + (ABS(bus) & LSR(bus)); }
        case 0x4F: { return cycles += 6 + (ABS(bus) & SRE(bus)); }

        case 0x50: { return cycles += 2 + (REL(bus) & BVC(bus)); }
        case 0x51: { return cycles += 5 + (IZY(bus) & EOR(bus)); }
        case 0x52: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x53: { return cycles += 8 + (IZY(bus) & SRE(bus)); }
        case 0x54: { return cycles += 4 + (ZPX(bus) & NOP(bus)); }
        case 0x55: { return cycles += 4 + (ZPX(bus) & EOR(bus)); }
        case 0x56: { return cycles += 6 + (ZPX(bus) & LSR(bus)); }
        case 0x57: { return cycles += 6 + (ZPX(bus) & SRE(bus)); }
        case 0x58: { return cycles += 2 + (IMP(bus) & CLI(bus)); }
        case 0x59: { return cycles += 4 + (ABY(bus) & EOR(bus)); }
        case 0x5A: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0x5B: { return cycles += 7 + (ABY(bus) & SRE(bus)); }
        case 0x5C: { return cycles += 4 + (ABX(bus) & NOP(bus)); }
        case 0x5D: { return cycles += 4 + (ABX(bus) & EOR(bus)); }
        case 0x5E: { return cycles += 7 + (ABX(bus) & LSR(bus)); }
        case 0x5F: { return cycles += 7 + (ABX(bus) & SRE(bus)); }

        case 0x60: { return cycles += 6 + (IMP(bus) & RTS(bus)); }
        case 0x61: { return cycles += 6 + (IZX(bus) & ADC(bus)); }
        case 0x62: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x63: { return cycles += 8 + (IZX(bus) & RRA(bus)); }
        case 0x64: { return cycles += 3 + (ZP0(bus) & NOP(bus)); }
        case 0x65: { return cycles += 3 + (ZP0(bus) & ADC(bus)); }
        case 0x66: { return cycles += 5 + (ZP0(bus) & ROR(bus)); }
        case 0x67: { return cycles += 5 + (ZP0(bus) & RRA(bus)); }
        case 0x68: { return cycles += 4 + (IMP(bus) & PLA(bus)); }
        case 0x69: { return cycles += 2 + (IMM(bus) & ADC(bus)); }
        case 0x6A: { return cycles += 2 + (IMP(bus) & ROR(bus)); }
        case 0x6B: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x6C: { return cycles += 5 + (IND(bus) & JMP(bus)); }
        case 0x6D: { return cycles += 4 + (ABS(bus) & ADC(bus)); }
        case 0x6E: { return cycles += 6 + (ABS(bus) & ROR(bus)); }
        case 0x6F: { return cycles += 6 + (ABS(bus) & RRA(bus)); }

        case 0x70: { return cycles += 2 + (REL(bus) & BVS(bus)); }
        case 0x71: { return cycles += 5 + (IZY(bus) & ADC(bus)); }
        case 0x72: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x73: { return cycles += 8 + (IZY(bus) & RRA(bus)); }
        case 0x74: { return cycles += 4 + (ZPX(bus) & NOP(bus)); }
        case 0x75: { return cycles += 4 + (ZPX(bus) & ADC(bus)); }
        case 0x76: { return cycles += 6 + (ZPX(bus) & ROR(bus)); }
        case 0x77: { return cycles += 6 + (ZPX(bus) & RRA(bus)); }
        case 0x78: { return cycles += 2 + (IMP(bus) & SEI(bus)); }
        case 0x79: { return cycles += 4 + (ABY(bus) & ADC(bus)); }
        case 0x7A: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0x7B: { return cycles += 7 + (ABY(bus) & RRA(bus)); }
        case 0x7C: { return cycles += 4 + (ABX(bus) & NOP(bus)); }
        case 0x7D: { return cycles += 4 + (ABX(bus) & ADC(bus)); }
        case 0x7E: { return cycles += 7 + (ABX(bus) & ROR(bus)); }
        case 0x7F: { return cycles += 7 + (ABX(bus) & RRA(bus)); }

        case 0x80: { return cycles += 2 + (IMM(bus) & NOP(bus)); }
        case 0x81: { return cycles += 6 + (IZX(bus) & STA(bus)); }
        case 0x82: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0x83: { return cycles += 6 + (IZX(bus) & SAX(bus)); }
        case 0x84: { return cycles += 3 + (ZP0(bus) & STY(bus)); }
        case 0x85: { return cycles += 3 + (ZP0(bus) & STA(bus)); }
        case 0x86: { return cycles += 3 + (ZP0(bus) & STX(bus)); }
        case 0x87: { return cycles += 3 + (ZP0(bus) & SAX(bus)); }
        case 0x88: { return cycles += 2 + (IMP(bus) & DEY(bus)); }
        case 0x89: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0x8A: { return cycles += 2 + (IMP(bus) & TXA(bus)); }
        case 0x8B: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x8C: { return cycles += 4 + (ABS(bus) & STY(bus)); }
        case 0x8D: { return cycles += 4 + (ABS(bus) & STA(bus)); }
        case 0x8E: { return cycles += 4 + (ABS(bus) & STX(bus)); }
        case 0x8F: { return cycles += 4 + (ABS(bus) & SAX(bus)); }

        case 0x90: { return cycles += 2 + (REL(bus) & BCC(bus)); }
        case 0x91: { return cycles += 6 + (IZY(bus) & STA(bus)); }
        case 0x92: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0x93: { return cycles += 6 + (IMP(bus) & XXX(bus)); }
        case 0x94: { return cycles += 4 + (ZPX(bus) & STY(bus)); }
        case 0x95: { return cycles += 4 + (ZPX(bus) & STA(bus)); }
        case 0x96: { return cycles += 4 + (ZPY(bus) & STX(bus)); }
        case 0x97: { return cycles += 4 + (ZPY(bus) & SAX(bus)); }
        case 0x98: { return cycles += 2 + (IMP(bus) & TYA(bus)); }
        case 0x99: { return cycles += 5 + (ABY(bus) & STA(bus)); }
        case 0x9A: { return cycles += 2 + (IMP(bus) & TXS(bus)); }
        case 0x9B: { return cycles += 5 + (IMP(bus) & XXX(bus)); }
        case 0x9C: { return cycles += 5 + (IMP(bus) & NOP(bus)); }
        case 0x9D: { return cycles += 5 + (ABX(bus) & STA(bus)); }
        case 0x9E: { return cycles += 5 + (IMP(bus) & XXX(bus)); }
        case 0x9F: { return cycles += 5 + (IMP(bus) & XXX(bus)); }

        case 0xA0: { return cycles += 2 + (IMM(bus) & LDY(bus)); }
        case 0xA1: { return cycles += 6 + (IZX(bus) & LDA(bus)); }
        case 0xA2: { return cycles += 2 + (IMM(bus) & LDX(bus)); }
        case 0xA3: { return cycles += 6 + (IZX(bus) & LAX(bus)); }
        case 0xA4: { return cycles += 3 + (ZP0(bus) & LDY(bus)); }
        case 0xA5: { return cycles += 3 + (ZP0(bus) & LDA(bus)); }
        case 0xA6: { return cycles += 3 + (ZP0(bus) & LDX(bus)); }
        case 0xA7: { return cycles += 3 + (ZP0(bus) & LAX(bus)); }
        case 0xA8: { return cycles += 2 + (IMP(bus) & TAY(bus)); }
        case 0xA9: { return cycles += 2 + (IMM(bus) & LDA(bus)); }
        case 0xAA: { return cycles += 2 + (IMP(bus) & TAX(bus)); }
        case 0xAB: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0xAC: { return cycles += 4 + (ABS(bus) & LDY(bus)); }
        case 0xAD: { return cycles += 4 + (ABS(bus) & LDA(bus)); }
        case 0xAE: { return cycles += 4 + (ABS(bus) & LDX(bus)); }
        case 0xAF: { return cycles += 4 + (ABS(bus) & LAX(bus)); }

        case 0xB0: { return cycles += 2 + (REL(bus) & BCS(bus)); }
        case 0xB1: { return cycles += 5 + (IZY(bus) & LDA(bus)); }
        case 0xB2: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0xB3: { return cycles += 5 + (IZY(bus) & LAX(bus)); }
        case 0xB4: { return cycles += 4 + (ZPX(bus) & LDY(bus)); }
        case 0xB5: { return cycles += 4 + (ZPX(bus) & LDA(bus)); }
        case 0xB6: { return cycles += 4 + (ZPY(bus) & LDX(bus)); }
        case 0xB7: { return cycles += 4 + (ZPY(bus) & LAX(bus)); }
        case 0xB8: { return cycles += 2 + (IMP(bus) & CLV(bus)); }
        case 0xB9: { return cycles += 4 + (ABY(bus) & LDA(bus)); }
        case 0xBA: { return cycles += 2 + (IMP(bus) & TSX(bus)); }
        case 0xBB: { return cycles += 4 + (IMP(bus) & XXX(bus)); }
        case 0xBC: { return cycles += 4 + (ABX(bus) & LDY(bus)); }
        case 0xBD: { return cycles += 4 + (ABX(bus) & LDA(bus)); }
        case 0xBE: { return cycles += 4 + (ABY(bus) & LDX(bus)); }
        case 0xBF: { return cycles += 4 + (ABY(bus) & LAX(bus)); }

        case 0xC0: { return cycles += 2 + (IMM(bus) & CPY(bus)); }
        case 0xC1: { return cycles += 6 + (IZX(bus) & CMP(bus)); }
        case 0xC2: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0xC3: { return cycles += 8 + (IZX(bus) & DCP(bus)); }
        case 0xC4: { return cycles += 3 + (ZP0(bus) & CPY(bus)); }
        case 0xC5: { return cycles += 3 + (ZP0(bus) & CMP(bus)); }
        case 0xC6: { return cycles += 5 + (ZP0(bus) & DEC(bus)); }
        case 0xC7: { return cycles += 5 + (ZP0(bus) & DCP(bus)); }
        case 0xC8: { return cycles += 2 + (IMP(bus) & INY(bus)); }
        case 0xC9: { return cycles += 2 + (IMM(bus) & CMP(bus)); }
        case 0xCA: { return cycles += 2 + (IMP(bus) & DEX(bus)); }
        case 0xCB: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0xCC: { return cycles += 4 + (ABS(bus) & CPY(bus)); }
        case 0xCD: { return cycles += 4 + (ABS(bus) & CMP(bus)); }
        case 0xCE: { return cycles += 6 + (ABS(bus) & DEC(bus)); }
        case 0xCF: { return cycles += 6 + (ABS(bus) & DCP(bus)); }

        case 0xD0: { return cycles += 2 + (REL(bus) & BNE(bus)); }
        case 0xD1: { return cycles += 5 + (IZY(bus) & CMP(bus)); }
        case 0xD2: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0xD3: { return cycles += 8 + (IZY(bus) & DCP(bus)); }
        case 0xD4: { return cycles += 4 + (ZPX(bus) & NOP(bus)); }
        case 0xD5: { return cycles += 4 + (ZPX(bus) & CMP(bus)); }
        case 0xD6: { return cycles += 6 + (ZPX(bus) & DEC(bus)); }
        case 0xD7: { return cycles += 6 + (ZPX(bus) & DCP(bus)); }
        case 0xD8: { return cycles += 2 + (IMP(bus) & CLD(bus)); }
        case 0xD9: { return cycles += 4 + (ABY(bus) & CMP(bus)); }
        case 0xDA: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0xDB: { return cycles += 7 + (ABY(bus) & DCP(bus)); }
        case 0xDC: { return cycles += 4 + (ABX(bus) & NOP(bus)); }
        case 0xDD: { return cycles += 4 + (ABX(bus) & CMP(bus)); }
        case 0xDE: { return cycles += 7 + (ABX(bus) & DEC(bus)); }
        case 0xDF: { return cycles += 7 + (ABX(bus) & DCP(bus)); }

        case 0xE0: { return cycles += 2 + (IMM(bus) & CPX(bus)); }
        case 0xE1: { return cycles += 6 + (IZX(bus) & SBC(bus)); }
        case 0xE2: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0xE3: { return cycles += 8 + (IZX(bus) & ISB(bus)); }
        case 0xE4: { return cycles += 3 + (ZP0(bus) & CPX(bus)); }
        case 0xE5: { return cycles += 3 + (ZP0(bus) & SBC(bus)); }
        case 0xE6: { return cycles += 5 + (ZP0(bus) & INC(bus)); }
        case 0xE7: { return cycles += 5 + (ZP0(bus) & ISB(bus)); }
        case 0xE8: { return cycles += 2 + (IMP(bus) & INX(bus)); }
        case 0xE9: { return cycles += 2 + (IMM(bus) & SBC(bus)); }
        case 0xEA: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0xEB: { return cycles += 2 + (IMM(bus) & SBC(bus)); }
        case 0xEC: { return cycles += 4 + (ABS(bus) & CPX(bus)); }
        case 0xED: { return cycles += 4 + (ABS(bus) & SBC(bus)); }
        case 0xEE: { return cycles += 6 + (ABS(bus) & INC(bus)); }
        case 0xEF: { return cycles += 6 + (ABS(bus) & ISB(bus)); }

        case 0xF0: { return cycles += 2 + (REL(bus) & BEQ(bus)); }
        case 0xF1: { return cycles += 5 + (IZY(bus) & SBC(bus)); }
        case 0xF2: { return cycles += 2 + (IMP(bus) & XXX(bus)); }
        case 0xF3: { return cycles += 8 + (IZY(bus) & ISB(bus)); }
        case 0xF4: { return cycles += 4 + (ZPX(bus) & NOP(bus)); }
        case 0xF5: { return cycles += 4 + (ZPX(bus) & SBC(bus)); }
        case 0xF6: { return cycles += 6 + (ZPX(bus) & INC(bus)); }
        case 0xF7: { return cycles += 6 + (ZPX(bus) & ISB(bus)); }
        case 0xF8: { return cycles += 2 + (IMP(bus) & SED(bus)); }
        case 0xF9: { return cycles += 4 + (ABY(bus) & SBC(bus)); }
        case 0xFA: { return cycles += 2 + (IMP(bus) & NOP(bus)); }
        case 0xFB: { return cycles += 7 + (ABY(bus) & ISB(bus)); }
        case 0xFC: { return cycles += 4 + (ABX(bus) & NOP(bus)); }
        case 0xFD: { return cycles += 4 + (ABX(bus) & SBC(bus)); }
        case 0xFE: { return cycles += 7 + (ABX(bus) & INC(bus)); }
        case 0xFF: { return cycles += 7 + (ABX(bus) & ISB(bus)); }
        default: { throw std::runtime_error("what opcode is this? ");
        }
    }
}

InstructionInfo CPU6502::instruction_info(uint8_t opcode) const {
    static const InstructionInfo table[256] = {
            //0x00
            {"BRK", "IMM", false}, 
            {"ORA", "IZX", false},
            {"???", "IMP", false},
            {"SLO", "IZX", true},
            {"NOP", "ZP0", true},
            {"ORA", "ZP0", false},
            {"ASL", "ZP0", false},
            {"SLO", "ZP0", true},
            {"PHP", "IMP", false},
            {"ORA", "IMM", false},
            {"ASL", "IMP", false},
            {"???", "IMP", false},
            {"NOP", "ABS", true},
            {"ORA", "ABS", false},
            {"ASL", "ABS", false},
            {"SLO", "ABS", true},
            //0x10
            {"BPL", "REL", false}, 
            {"ORA", "IZY", false},
            {"???", "IMP", false},
            {"SLO", "IZY", true},
            {"NOP", "ZPX", true},
            {"ORA", "ZPX", false},
            {"ASL", "ZPX", false},
            {"SLO", "ZPX", true},
            {"CLC", "IMP", false},
            {"ORA", "ABY", false},
            {"NOP", "IMP", true},
            {"SLO", "ABY", true},
            {"NOP", "ABX", true},
            {"ORA", "ABX", false},
            {"ASL", "ABX", false},
            {"SLO", "ABX", true},
            //0x20
            {"JSR", "ABS", false},
            {"AND", "IZX", false},
            {"???", "IMP", false},
            {"RLA", "IZX", true},
            {"BIT", "ZP0", false},
            {"AND", "ZP0", false},
            {"ROL", "ZP0", false},
            {"RLA", "ZP0", true},
            {"PLP", "IMP", false},
            {"AND", "IMM", false},
            {"ROL", "IMP", false},
            {"ANC", "IMM", true},
            {"BIT", "ABS", false},
            {"AND", "ABS", false},
            {"ROL", "ABS", false},
            {"RLA", "ABS", true},
            //0x30
            {"BMI", "REL", false},
            {"AND", "IZY", false},
            {"???", "IMP", false},
            {"RLA", "IZY", true},
            {"NOP", "ZPX", true},
            {"AND", "ZPX", false},
            {"ROL", "ZPX", false},
            {"RLA", "ZPX", true},
            {"SEC", "IMP", false},
            {"AND", "ABY", false},
            {"NOP", "IMP", true},
            {"RLA", "ABY", true},
            {"NOP", "ABX", true},
            {"AND", "ABX", false},
            {"ROL", "ABX", false},
            {"RLA", "ABX", true},
            //0x40
            {"RTI", "IMP", false},
            {"EOR", "IZX", false},
            {"???", "IMP", false},
            {"SRE", "IZX", true},
            {"NOP", "ZP0", true},
            {"EOR", "ZP0", false},
            {"LSR", "ZP0", false},
            {"SRE", "ZP0", true},
            {"PHA", "IMP", false},
            {"EOR", "IMM", false},
            {"LSR", "IMP", false},
            {"???", "IMP", false},
            {"JMP", "ABS", false},
            {"EOR", "ABS", false},
            {"LSR", "ABS", false},
            {"SRE", "ABS", true},
            //0x50
            {"BVC", "REL", false},
            {"EOR", "IZY", false},
            {"???", "IMP", false},
            {"SRE", "IZY", true},
            {"NOP", "ZPX", true},
            {"EOR", "ZPX", false},
            {"LSR", "ZPX", false},
            {"SRE", "ZPX", true},
            {"CLI", "IMP", false},
            {"EOR", "ABY", false},
            {"NOP", "IMP", true},
            {"SRE", "ABY", true},
            {"NOP", "ABX", true},
            {"EOR", "ABX", false},
            {"LSR", "ABX", false},
            {"SRE", "ABX", true},
            //0x60
            {"RTS", "IMP", false},
            {"ADC", "IZX", false},
            {"???", "IMP", false},
            {"RRA", "IZX", true},
            {"NOP", "ZP0", true},
            {"ADC", "ZP0", false},
            {"ROR", "ZP0", false},
            {"RRA", "ZP0", true},
            {"PLA", "IMP", false},
            {"ADC", "IMM", false},
            {"ROR", "IMP", false},
            {"???", "IMP", false},
            {"JMP", "IND", false},
            {"ADC", "ABS", false},
            {"ROR", "ABS", false},
            {"RRA", "ABS", true},
            //0x70
            {"BVS", "REL", false},
            {"ADC", "IZY", false},
            {"???", "IMP", false},
            {"RRA", "IZY", true},
            {"NOP", "ZPX", true},
            {"ADC", "ZPX", false},
            {"ROR", "ZPX", false},
            {"RRA", "ZPX", true},
            {"SEI", "IMP", false},
            {"ADC", "ABY", false},
            {"NOP", "IMP", true},
            {"RRA", "ABY", true},
            {"NOP", "ABX", true},
            {"ADC", "ABX", false},
            {"ROR", "ABX", false},
            {"RRA", "ABX", true},
            //0x80
            {"NOP", "IMM", true},
            {"STA", "IZX", false},
            {"???", "IMP", false},
            {"SAX", "IZX", true},
            {"STY", "ZP0", false},
            {"STA", "ZP0", false},
            {"STX", "ZP0", false},
            {"SAX", "ZP0", true},
            {"DEY", "IMP", false},
            {"???", "IMP", false},
            {"TXA", "IMP", false},
            {"???", "IMP", false},
            {"STY", "ABS", false},
            {"STA", "ABS", false},
            {"STX", "ABS", false},
            {"SAX", "ABS", true},
            //0x90
            {"BCC", "REL", false},
            {"STA", "IZY", false},
            {"???", "IMP", false},
            {"???", "IMP", false},
            {"STY", "ZPX", false},
            {"STA", "ZPX", false},
            {"STX", "ZPY", false},
            {"SAX", "ZPY", true},
            {"TYA", "IMP", false},
            {"STA", "ABY", false},
            {"TXS", "IMP", false},
            {"???", "IMP", false},
            {"???", "IMP", false},
            {"STA", "ABX", false},
            {"???", "IMP", false},
            {"???", "IMP", false},
            //0xA0
            {"LDY", "IMM", false},
            {"LDA", "IZX", false},
            {"LDX", "IMM", false},
            {"LAX", "IZX", true},
            {"LDY", "ZP0", false},
            {"LDA", "ZP0", false},
            {"LDX", "ZP0", false},
            {"LAX", "ZP0", true},
            {"TAY", "IMP", false},
            {"LDA", "IMM", false},
            {"TAX", "IMP", false},
            {"???", "IMP", false},
            {"LDY", "ABS", false},
            {"LDA", "ABS", false},
            {"LDX", "ABS", false},
            {"LAX", "ABS", true},
            //0xB0
            {"BCS", "REL", false},
            {"LDA", "IZY", false},
            {"???", "IMP", false},
            {"LAX", "IZY", true},
            {"LDY", "ZPX", false},
            {"LDA", "ZPX", false},
            {"LDX", "ZPY", false},
            {"LAX", "ZPY", true},
            {"CLV", "IMP", false},
            {"LDA", "ABY", false},
            {"TSX", "IMP", false},
            {"???", "IMP", false},
            {"LDY", "ABX", false},
            {"LDA", "ABX", false},
            {"LDX", "ABY", false},
            {"LAX", "ABY", true},
            //0xC0
            {"CPY", "IMM", false},
            {"CMP", "IZX", false},
            {"???", "IMP", false},
            {"DCP", "IZX", true},
            {"CPY", "ZP0", false},
            {"CMP", "ZP0", false},
            {"DEC", "ZP0", false},
            {"DCP", "ZP0", true},
            {"INY", "IMP", false},
            {"CMP", "IMM", false},
            {"DEX", "IMP", false},
            {"???", "IMP", false},
            {"CPY", "ABS", false},
            {"CMP", "ABS", false},
            {"DEC", "ABS", false},
            {"DCP", "ABS", true},
            //0xD0
            {"BNE", "REL", false},
            {"CMP", "IZY", false},
            {"???", "IMP", false},
            {"DCP", "IZY", true},
            {"NOP", "ZPX", true},
            {"CMP", "ZPX", false},
            {"DEC", "ZPX", false},
            {"DCP", "ZPX", true},
            {"CLD", "IMP", false},
            {"CMP", "ABY", false},
            {"NOP", "IMP", true},
            {"DCP", "ABY", true},
            {"NOP", "ABX", true},
            {"CMP", "ABX", false},
            {"DEC", "ABX", false},
            {"DCP", "ABX", true},
            //0xE0
            {"CPX", "IMM", false},
            {"SBC", "IZX", false},
            {"???", "IMP", false},
            {"ISB", "IZX", true},
            {"CPX", "ZP0", false},
            {"SBC", "ZP0", false},
            {"INC", "ZP0", false},
            {"ISB", "ZP0", true},
            {"INX", "IMP", false},
            {"SBC", "IMM", false},
            {"NOP", "IMP", false},
            {"SBC", "IMM", true},
            {"CPX", "ABS", false},
            {"SBC", "ABS", false},
            {"INC", "ABS", false},
            {"ISB", "ABS", true},
            //0xF0
            {"BEQ", "REL", false},
            {"SBC", "IZY", false},
            {"???", "IMP", false},
            {"ISB", "IZY", true},
            {"NOP", "ZPX", true},
            {"SBC", "ZPX", false},
            {"INC", "ZPX", false},
            {"ISB", "ZPX", true},
            {"SED", "IMP", false},
            {"SBC", "ABY", false},
            {"NOP", "IMP", true},
            {"ISB", "ABY", true},
            {"NOP", "ABX", true},
            {"SBC", "ABX", false},
            {"INC", "ABX", false},
            {"ISB", "ABX", true},

    };
    return table[opcode];
}

/// Implied -> the fetched value is the _a_ register
uint8_t CPU6502::IMP(CPUIO &bus) {
    implied = a;
    implied_has_value = true;
    return 0;
}

/// Immediate -> the value pointed to by the program counter
uint8_t CPU6502::IMM(CPUIO &bus) {
    addr_abs = pc;
    pc += 1;
    return 0;
}

/// Zero Page 0 -> The address is in the range $0000-$00FF
uint8_t CPU6502::ZP0(CPUIO &bus) {
    addr_abs = bus.read(pc);
    pc += 1;
    return 0;
}

/// Zero Page + X -> An address is read from memory and _x_ is added.
/// The final address is in the range $0000-$00FF (wrap around).
uint8_t CPU6502::ZPX(CPUIO &bus) {
    addr_abs = bus.read(pc) + x;
    addr_abs &= 0x00FF;
    pc += 1;
    return 0;
}

/// Zero Page + Y -> An address is read from memory and _y_ is added.
/// The final address is in the range $0000-$00FF (wrap around).
uint8_t CPU6502::ZPY(CPUIO &bus) {
    addr_abs = bus.read(pc) + y;
    addr_abs &= 0x00FF;
    pc += 1;
    return 0;
}

/// Relative -> A value is read from memory and stored for branching jump operations.
uint8_t CPU6502::REL(CPUIO &bus) {
    addr_rel = bus.read(pc);
    pc += 1;
    return 0;
}

/// Absolute -> Two values are read from memory and used as an address.
uint8_t CPU6502::ABS(CPUIO &bus) {
    auto lo = bus.read(pc);
    pc += 1;
    auto hi = bus.read(pc);
    pc += 1;
    addr_abs = (hi << 8) | lo;
    return 0;
}

/// Absolute + x -> Two values are read from memory then _x_ is added
/// and used as an address.
uint8_t CPU6502::ABX(CPUIO &bus) {
    auto lo = bus.read(pc);
    pc += 1;
    auto hi = bus.read(pc);
    pc += 1;
    addr_abs = (hi << 8) | lo;
    addr_abs += x;

    // Cycle count increased if crossing a page
    if ((addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    } else {
        return 0;
    }
}

/// Absolute + y -> Two values are read from memory then _y_ is added
/// and used as an address.
uint8_t CPU6502::ABY(CPUIO &bus) {
    auto lo = bus.read(pc);
    pc += 1;
    auto hi = bus.read(pc);
    pc += 1;
    addr_abs = (hi << 8) | lo;
    addr_abs = addr_abs + y;

    // Cycle count increased if crossing a page
    if ((addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    } else {
        return 0;
    }
}

/// Indirect -> Two values are read from memory and used as an address to
/// read the actual address.
uint8_t CPU6502::IND(CPUIO &bus) {
    auto ptr_lo = bus.read(pc);
    pc += 1;
    auto ptr_hi = bus.read(pc);
    pc += 1;
    auto ptr = (ptr_hi << 8) | ptr_lo;

    // Simulate page boundary hardware bug
    if (ptr_lo == 0x00FF) {
        auto hi = bus.read(ptr & 0xFF00);
        auto lo = bus.read(ptr);
        addr_abs = (hi << 8) | lo;
    } else {
        auto hi = bus.read(ptr + 1);
        auto lo = bus.read(ptr);
        addr_abs = (hi << 8) | lo;
    }
    return 0;
}

/// Indirect Zero Page + X -> A value is read from memory and _x_ is added, then
/// the actual address is read from the zero page.
uint8_t CPU6502::IZX(CPUIO &bus) {
    auto t = bus.read(pc);
    pc += 1;

    auto lo = bus.read((t + x) & 0x00FF);
    auto hi = bus.read((t + x + 1) & 0x00FF);
    addr_abs = (hi << 8) | lo;

    return 0;
}

/// Indirect Zero Page + Y -> A value is read from memory and the actual
/// address is read from the zero page then _y_ is added.
uint8_t CPU6502::IZY(CPUIO &bus) {
    auto t = bus.read(pc);
    pc += 1;

    auto lo = bus.read(t & 0x00FF);
    auto hi = bus.read((t + 1) & 0x00FF);
    auto addr = ((hi << 8) | lo) + y;
    addr_abs = addr;

    // Cycle count increased if crossing a page
    if ((addr & 0xFF00) != (hi << 8)) {
        return 1;
    } else {
        return 0;
    }
}

/// checks if bit 7 is set -> negative number
bool CPU6502::is_negative(uint8_t value) {
    return (value & 0x80) == 0x80;
}

/// the actual add operation with carry and overflow checking
void CPU6502::add(uint8_t m) {
    auto carry = get_flag(Flags6502::C);
    uint16_t temp = a + m + carry;
    set_status_flag(Flags6502::C, temp > 0xFF);
    set_status_flag(Flags6502::Z, (temp & 0x00FF) == 0);
    set_status_flag(Flags6502::N, is_negative(temp));
    auto overflow = (~(a ^ m) & (a ^ temp)) & 0x0080;
    set_status_flag(Flags6502::V, overflow == 0x0080);
    a = temp & 0x00FF;
}

/// the actual decimal add operation with carry and overflow checking
void CPU6502::add_bdc(uint8_t m, int8_t sign) {
    int8_t carry = get_flag(Flags6502::C);
    if(sign < 0) {
        carry -= 1;
    }
    auto d1_lo = a & 0x0F;
    auto d1_hi = (a & 0xF0) >> 4;
    auto d2_lo = m & 0x0F;
    auto d2_hi = (m & 0xF0) >> 4;
    int8_t t1 = d1_lo + sign * d2_lo + carry;
    if(0 > t1 || t1 > 9) {
        t1 += sign * 6;
        carry = sign;
    } else {
        carry = 0;
    }
    uint8_t t2 = d1_hi + sign * d2_hi + carry;
    carry = 0;
    if(0 > t2 || t2 > 9) {
        t2 += sign * 6;
        carry = sign;
    }

    uint8_t temp = (t2 & 0x0F) << 4 | (t1 & 0x0F);

    if(sign < 0) {
        carry += 1;
    }

    set_status_flag(Flags6502::C, carry);
    set_status_flag(Flags6502::Z, temp == 0);
    a = temp;
}

/// Add with carry
uint8_t CPU6502::ADC(CPUIO &bus) {
    auto m = fetch(bus);
    if(is_status_flag_set(Flags6502::D)) {
        add_bdc(m, 1);
    } else {
        add(m);
    }
    return 1;
}

/// ANDs the contents of the A register with an immediate
/// value and then moves bit 7 of A into the Carry flag
uint8_t CPU6502::ANC(CPUIO &bus) {
    AND(bus);
    ASL(bus);
    return 1;
}

/// AND (with accumulator)
uint8_t CPU6502::AND(CPUIO &bus) {
    auto m = fetch(bus);
    a = a & m;
    set_status_flag(Flags6502::Z, a == 0x00);
    set_status_flag(Flags6502::N, is_negative(a));
    return 1;
}

/// arithmetic shift left
uint8_t CPU6502::ASL(CPUIO &bus) {
    auto m = fetch(bus);
    set_status_flag(Flags6502::C, (m & 0x80) == 0x80);
    m = m << 1;
    set_status_flag(Flags6502::Z, m == 0x00);
    set_status_flag(Flags6502::N, is_negative(m));

    if (implied_has_value) {
        a = m;
    } else {
        bus.write(addr_abs, m);
    }
    return 0;
}


/// common function for the branching instructions
void CPU6502::branch() {
    cycles += 1;
    addr_abs = pc + (int8_t) addr_rel;

    if ((addr_abs & 0xFF00) != (pc & 0xFF00)) {
        cycles += 1;
    }

    pc = addr_abs;
}

/// branch on carry clear
uint8_t CPU6502::BCC(CPUIO &bus) {
    if (!is_status_flag_set(Flags6502::C)) {
        branch();
    }
    return 0;
}

/// branch on carry set
uint8_t CPU6502::BCS(CPUIO &bus) {
    if (is_status_flag_set(Flags6502::C)) {
        branch();
    }
    return 0;
}

/// branch on equal (zero set)
uint8_t CPU6502::BEQ(CPUIO &bus) {
    if (is_status_flag_set(Flags6502::Z)) {
        branch();
    }
    return 0;
}

/// bit test
uint8_t CPU6502::BIT(CPUIO &bus) {
    auto m = fetch(bus);
    auto temp = a & m;
    set_status_flag(Flags6502::Z, temp == 0x00);
    set_status_flag(Flags6502::N, is_flag_set(Flags6502::N, m));
    set_status_flag(Flags6502::V, is_flag_set(Flags6502::V, m));
    return 0;
}

/// branch on minus (negative set)
uint8_t CPU6502::BMI(CPUIO &bus) {
    if (is_status_flag_set(Flags6502::N)) {
        branch();
    }
    return 0;
}

/// branch on not equal (zero clear)
uint8_t CPU6502::BNE(CPUIO &bus) {
    if (!is_status_flag_set(Flags6502::Z)) {
        branch();
    }
    return 0;
}

/// branch on plus (negative clear)
uint8_t CPU6502::BPL(CPUIO &bus) {
    if (!is_status_flag_set(Flags6502::N)) {
        branch();
    }
    return 0;
}

/// break / interrupt
uint8_t CPU6502::BRK(CPUIO &bus) {
    // pc += 1;
    push_program_counter_on_stack(bus);
    push_value_on_stack(bus, status | Flags6502::U | Flags6502::B);
    load_program_counter_from_addr(bus, 0xFFFE);
    set_status_flag(Flags6502::I, true);

    return 0;
}

/// branch on overflow clear
uint8_t CPU6502::BVC(CPUIO &bus) {
    if (!is_status_flag_set(Flags6502::V)) {
        branch();
    }
    return 0;
}

/// branch on overflow set
uint8_t CPU6502::BVS(CPUIO &bus) {
    if (is_status_flag_set(Flags6502::V)) {
        branch();
    }
    return 0;
}

/// clear carry
uint8_t CPU6502::CLC(CPUIO &bus) {
    set_status_flag(Flags6502::C, false);
    return 0;
}

/// clear decimal
uint8_t CPU6502::CLD(CPUIO &bus) {
    set_status_flag(Flags6502::D, false);
    return 0;
}

/// clear interrupt disable
uint8_t CPU6502::CLI(CPUIO &bus) {
    set_status_flag(Flags6502::I, false);
    return 0;
}

/// clear overflow
uint8_t CPU6502::CLV(CPUIO &bus) {
    set_status_flag(Flags6502::V, false);
    return 0;
}

/// compare (with accumulator)
uint8_t CPU6502::CMP(CPUIO &bus) {
    auto m = fetch(bus);
    auto temp = a - m;
    set_status_flag(Flags6502::C, a >= m);
    set_status_flag(Flags6502::Z, temp == 0);
    set_status_flag(Flags6502::N, is_negative(temp));
    return 1;
}

/// compare with X
uint8_t CPU6502::CPX(CPUIO &bus) {
    auto m = fetch(bus);
    auto temp = x - m;
    set_status_flag(Flags6502::C, x >= m);
    set_status_flag(Flags6502::Z, temp == 0);
    set_status_flag(Flags6502::N, is_negative(temp));
    return 1;
}

/// compare with Y
uint8_t CPU6502::CPY(CPUIO &bus) {
    auto m = fetch(bus);
    auto temp = y - m;
    set_status_flag(Flags6502::C, y >= m);
    set_status_flag(Flags6502::Z, temp == 0);
    set_status_flag(Flags6502::N, is_negative(temp));
    return 1;
}

/// decrement and compare with Accumulator
uint8_t CPU6502::DCP(CPUIO &bus) {
    DEC(bus);
    CMP(bus);
    return 1;
}

/// decrement
uint8_t CPU6502::DEC(CPUIO &bus) {
    auto m = fetch(bus);
    m = m - 0x01;
    bus.write(addr_abs, m);
    set_status_flag(Flags6502::Z, m == 0x00);
    set_status_flag(Flags6502::N, is_negative(m));
    return 0;
}

/// decrement X
uint8_t CPU6502::DEX(CPUIO &bus) {
    x = x - 0x01;
    set_status_flag(Flags6502::Z, x == 0x00);
    set_status_flag(Flags6502::N, is_negative(x));
    return 0;
}

/// decrement Y
uint8_t CPU6502::DEY(CPUIO &bus) {
    y = y - 0x01;
    set_status_flag(Flags6502::Z, y == 0x00);
    set_status_flag(Flags6502::N, is_negative(y));
    return 0;
}

/// exclusive or (with accumulator)
uint8_t CPU6502::EOR(CPUIO &bus) {
    auto m = fetch(bus);
    a ^= m;
    set_status_flag(Flags6502::Z, a == 0);
    set_status_flag(Flags6502::N, is_negative(a));
    return 1;
}

/// increment
uint8_t CPU6502::INC(CPUIO &bus) {
    auto m = fetch(bus);
    m = m + 1;
    bus.write(addr_abs, m);
    set_status_flag(Flags6502::Z, m == 0x00);
    set_status_flag(Flags6502::N, is_negative(m));
    return 0;
}

/// increment X
uint8_t CPU6502::INX(CPUIO &bus) {
    x = x + 0x01;
    set_status_flag(Flags6502::Z, x == 0x00);
    set_status_flag(Flags6502::N, is_negative(x));
    return 0;
}

/// increment Y
uint8_t CPU6502::INY(CPUIO &bus) {
    y = y + 0x01;
    set_status_flag(Flags6502::Z, y == 0x00);
    set_status_flag(Flags6502::N, is_negative(y));
    return 0;
}

/// Increment memory with one and then subtract memory from Accumulator
uint8_t CPU6502::ISB(CPUIO &bus) {
    INC(bus);
    SBC(bus);
    return 1;
}

/// jump
uint8_t CPU6502::JMP(CPUIO &bus) {
    pc = addr_abs;
    return 0;
}

/// jump subroutine
uint8_t CPU6502::JSR(CPUIO &bus) {
    pc -= 1;
    push_program_counter_on_stack(bus);
    pc = addr_abs;
    return 0;
}

/// * load accumulator and x with memory contents
uint8_t CPU6502::LAX(CPUIO &bus) {
    auto m = fetch(bus);
    a = m;
    x = m;
    set_status_flag(Flags6502::Z, m == 0);
    set_status_flag(Flags6502::N, is_negative(m));
    return 1;
}

/// load accumulator
uint8_t CPU6502::LDA(CPUIO &bus) {
    a = fetch(bus);
    set_status_flag(Flags6502::Z, a == 0x00);
    set_status_flag(Flags6502::N, is_negative(a));
    return 1;
}

/// Load X
uint8_t CPU6502::LDX(CPUIO &bus) {
    x = fetch(bus);
    set_status_flag(Flags6502::Z, x == 0x00);
    set_status_flag(Flags6502::N, is_negative(x));
    return 1;
}

/// Load Y
uint8_t CPU6502::LDY(CPUIO &bus) {
    y = fetch(bus);
    set_status_flag(Flags6502::Z, y == 0x00);
    set_status_flag(Flags6502::N, is_negative(y));
    return 1;
}

/// logical shift right
uint8_t CPU6502::LSR(CPUIO &bus) {
    auto m = fetch(bus);
    set_status_flag(Flags6502::C, (m & 0x01) == 0x01);
    m = m >> 1;
    set_status_flag(Flags6502::Z, m == 0);
    set_status_flag(Flags6502::N, false);

    if (implied_has_value) {
        a = m;
    } else {
        bus.write(addr_abs, m);
    }
    return 0;
}

/// no operation
uint8_t CPU6502::NOP(CPUIO &bus) {
    switch (opcode) {
        case (0x1C):
        case (0x3C):
        case (0x5C):
        case (0x7C):
        case (0xDC):
        case (0xFC):return 1;
        default:return 0;
    }
}

/// or with accumulator
uint8_t CPU6502::ORA(CPUIO &bus) {
    auto m = fetch(bus);
    a |= m;
    set_status_flag(Flags6502::Z, a == 0);
    set_status_flag(Flags6502::N, is_negative(a));
    return 1;
}

/// push accumulator
uint8_t CPU6502::PHA(CPUIO &bus) {
    push_value_on_stack(bus, a);
    return 0;
}

/// push processor status (SR)
uint8_t CPU6502::PHP(CPUIO &bus) {
    push_value_on_stack(bus, status | Flags6502::B | Flags6502::U);
    return 0;
}

/// pull accumulator
uint8_t CPU6502::PLA(CPUIO &bus) {
    a = pop_value_from_stack(bus);
    set_status_flag(Flags6502::Z, a == 0x00);
    set_status_flag(Flags6502::N, is_negative(a));
    return 0;
}

/// pull processor status (SR)
uint8_t CPU6502::PLP(CPUIO &bus) {
    status = pop_value_from_stack(bus);
    set_status_flag(Flags6502::U, true);
    set_status_flag(Flags6502::B, false);
    return 0;
}

/// rotate memory left and then AND accumulator with memory
uint8_t CPU6502::RLA(CPUIO &bus) {
    ROL(bus);
    AND(bus);
    return 0;
}

/// rotate left
uint8_t CPU6502::ROL(CPUIO &bus) {
    uint16_t m = fetch(bus);
    m = (m << 1) | get_flag(Flags6502::C);
    set_status_flag(Flags6502::C, (m & 0xFF00) != 0);

    m &= 0x00FF;
    set_status_flag(Flags6502::Z, m == 0);
    set_status_flag(Flags6502::N, is_negative(m));

    if (implied_has_value) {
        a = m;
    } else {
        bus.write(addr_abs, m);
    }

    return 0;
}

/// rotate right
uint8_t CPU6502::ROR(CPUIO &bus) {
    uint16_t m = fetch(bus);
    auto carry = m & 0x01;
    m = ((get_flag(Flags6502::C)) << 7) | (m >> 1);

    m &= 0x00FF;
    set_status_flag(Flags6502::C, carry != 0);
    set_status_flag(Flags6502::Z, m == 0);
    set_status_flag(Flags6502::N, is_negative(m));

    if (implied_has_value) {
        a = m;
    } else {
        bus.write(addr_abs, m);
    }

    return 0;
}

/// Rotate memory one bit to the right and then add memory to accumulator
uint8_t CPU6502::RRA(CPUIO &bus) {
    ROR(bus);
    ADC(bus);
    return 0;
}

/// return from interrupt
uint8_t CPU6502::RTI(CPUIO &bus) {
    status = pop_value_from_stack(bus);
    set_status_flag(Flags6502::U, true);
    set_status_flag(Flags6502::B, false);
    pop_program_counter_from_stack(bus);
    return 0;
}

/// return from subroutine
uint8_t CPU6502::RTS(CPUIO &bus) {
    pop_program_counter_from_stack(bus);
    pc += 1;
    return 0;
}

/// AND X with Accumulator and store in memory
uint8_t CPU6502::SAX(CPUIO &bus) {
    auto temp = a & x;
    bus.write(addr_abs, temp);
    return 0;
}

/// Subtract with borrow
uint8_t CPU6502::SBC(CPUIO &bus) {
    auto m = fetch(bus);
    if(is_status_flag_set(Flags6502::D)) {
        add_bdc(m, -1);
    } else {
        add(~m);
    }
    return 1;
}

/// Set Carry
uint8_t CPU6502::SEC(CPUIO &bus) {
    set_status_flag(Flags6502::C, true);
    return 0;
}

/// Set Decimal
uint8_t CPU6502::SED(CPUIO &bus) {
    set_status_flag(Flags6502::D, true);
    return 0;
}

/// set interrupt disable
uint8_t CPU6502::SEI(CPUIO &bus) {
    set_status_flag(Flags6502::I, true);
    return 0;
}

/// Shift memory left and then OR accumulator with memory
uint8_t CPU6502::SLO(CPUIO &bus) {
    ASL(bus);
    ORA(bus);
    return 0;
}

/// Shift memory one bit right then XOR with accumulator
uint8_t CPU6502::SRE(CPUIO &bus) {
    LSR(bus);
    EOR(bus);
    return 0;
}

/// store accumulator
uint8_t CPU6502::STA(CPUIO &bus) {
    bus.write(addr_abs, a);
    return 0;
}

/// store X
uint8_t CPU6502::STX(CPUIO &bus) {
    bus.write(addr_abs, x);
    return 0;
}

/// store Y
uint8_t CPU6502::STY(CPUIO &bus) {
    bus.write(addr_abs, y);
    return 0;
}

/// transfer accumulator to X
uint8_t CPU6502::TAX(CPUIO &bus) {
    x = a;
    set_status_flag(Flags6502::Z, x == 0x00);
    set_status_flag(Flags6502::N, is_negative(x));
    return 0;
}

/// transfer accumulator to Y
uint8_t CPU6502::TAY(CPUIO &bus) {
    y = a;
    set_status_flag(Flags6502::Z, y == 0x00);
    set_status_flag(Flags6502::N, is_negative(y));
    return 0;
}

/// transfer stack pointer to X
uint8_t CPU6502::TSX(CPUIO &bus) {
    x = stkp;
    set_status_flag(Flags6502::Z, x == 0x00);
    set_status_flag(Flags6502::N, is_negative(x));
    return 0;
}

/// transfer X to accumulator
uint8_t CPU6502::TXA(CPUIO &bus) {
    a = x;
    set_status_flag(Flags6502::Z, a == 0x00);
    set_status_flag(Flags6502::N, is_negative(a));
    return 0;
}

/// transfer X to stack pointer
uint8_t CPU6502::TXS(CPUIO &bus) {
    stkp = x;
    return 0;
}

/// transfer Y to accumulator
uint8_t CPU6502::TYA(CPUIO &bus) {
    a = y;
    set_status_flag(Flags6502::Z, a == 0x00);
    set_status_flag(Flags6502::N, is_negative(a));
    return 0;
}


uint8_t CPU6502::XXX(CPUIO &bus) {
    printf("Illegal opcode %02X at $%04X\n", opcode, pc);
    throw std::runtime_error("err");
}
