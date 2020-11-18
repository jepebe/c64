#include "catch2.hpp"
#include "common.hpp"
#include "c64/cpu_6502.hpp"

std::string addressing_mode_name(uint8_t opcode);
std::string instruction_name(uint8_t opcode);

TEST_CASE("CPU") {
    auto cpu = CPU6502();
    auto bus = MockBus();

    SECTION("CPU flags") {
        auto flag = Flags6502::C;
        REQUIRE(flag == 0x01);
        auto status = Flags6502::B | Flags6502::V;
        REQUIRE(status == 0x50);
        status |= Flags6502::C;
        REQUIRE(status == 0x51);
    }

    SECTION("CPU reset") {
        bus.write(0xFFFC, 0xA5);
        bus.write(0xFFFD, 0x7F);

        REQUIRE(cpu.pc == 0x0000);
        cpu.reset(bus);
        REQUIRE(cpu.pc == 0x7FA5);
    }

    SECTION("CPU clock") {
        cpu.reset(bus);
        REQUIRE(cpu.cycles == 8);
        cpu.clock(bus);
        REQUIRE(cpu.cycles == 7);
    }

    SECTION("CPU Addressing Mode Names") {
        bool all_success = true;
        uint8_t count = 0;
        for (uint16_t opcode = 0x00; opcode <= 0xFF; opcode++) {
            auto addr_mode = addressing_mode_name(opcode);
            auto cpu_addr_mode = cpu.instruction_info(opcode).addr_mode;
            if (addr_mode != cpu_addr_mode) {
                printf("For opcode %02X modes differ: %s != %s\n", opcode,
                       addr_mode.c_str(), cpu_addr_mode);
                all_success = false;
                count++;
            }
        }
        if (!all_success) {
            printf("Failed addressing modes: %d\n", count);
        }
        REQUIRE(all_success == true);
    }

    SECTION("CPU Instruction Names") {
        bool all_success = true;
        uint8_t count = 0;
        for (uint16_t opcode = 0x00; opcode <= 0xFF; opcode++) {
            auto inst_name = instruction_name(opcode);
            auto cpu_inst_name = cpu.instruction_info(opcode).instruction;
            if (inst_name != cpu_inst_name) {
                printf("For opcode %02X instructions differ: %s != %s\n", opcode,
                       inst_name.c_str(), cpu_inst_name);
                all_success = false;
                count++;
            }
        }
        if (!all_success) {
            printf("Failed instructions: %d\n", count);
        }
        REQUIRE(all_success == true);
    }
}

std::string addressing_mode_name(uint8_t opcode) {
    switch (opcode) {
        case 0x00: { return "IMM"; }
        case 0x01: { return "IZX"; }
        case 0x02: { return "IMP"; }
        case 0x03: { return "IZX"; }
        case 0x04: { return "ZP0"; }
        case 0x05: { return "ZP0"; }
        case 0x06: { return "ZP0"; }
        case 0x07: { return "ZP0"; }
        case 0x08: { return "IMP"; }
        case 0x09: { return "IMM"; }
        case 0x0A: { return "IMP"; }
        case 0x0B: { return "IMP"; }
        case 0x0C: { return "ABS"; }
        case 0x0D: { return "ABS"; }
        case 0x0E: { return "ABS"; }
        case 0x0F: { return "ABS"; }

        case 0x10: { return "REL"; }
        case 0x11: { return "IZY"; }
        case 0x12: { return "IMP"; }
        case 0x13: { return "IZY"; }
        case 0x14: { return "ZPX"; }
        case 0x15: { return "ZPX"; }
        case 0x16: { return "ZPX"; }
        case 0x17: { return "ZPX"; }
        case 0x18: { return "IMP"; }
        case 0x19: { return "ABY"; }
        case 0x1A: { return "IMP"; }
        case 0x1B: { return "ABY"; }
        case 0x1C: { return "ABX"; }
        case 0x1D: { return "ABX"; }
        case 0x1E: { return "ABX"; }
        case 0x1F: { return "ABX"; }

        case 0x20: { return "ABS"; }
        case 0x21: { return "IZX"; }
        case 0x22: { return "IMP"; }
        case 0x23: { return "IZX"; }
        case 0x24: { return "ZP0"; }
        case 0x25: { return "ZP0"; }
        case 0x26: { return "ZP0"; }
        case 0x27: { return "ZP0"; }
        case 0x28: { return "IMP"; }
        case 0x29: { return "IMM"; }
        case 0x2A: { return "IMP"; }
        case 0x2B: { return "IMM"; }
        case 0x2C: { return "ABS"; }
        case 0x2D: { return "ABS"; }
        case 0x2E: { return "ABS"; }
        case 0x2F: { return "ABS"; }

        case 0x30: { return "REL"; }
        case 0x31: { return "IZY"; }
        case 0x32: { return "IMP"; }
        case 0x33: { return "IZY"; }
        case 0x34: { return "ZPX"; }
        case 0x35: { return "ZPX"; }
        case 0x36: { return "ZPX"; }
        case 0x37: { return "ZPX"; }
        case 0x38: { return "IMP"; }
        case 0x39: { return "ABY"; }
        case 0x3A: { return "IMP"; }
        case 0x3B: { return "ABY"; }
        case 0x3C: { return "ABX"; }
        case 0x3D: { return "ABX"; }
        case 0x3E: { return "ABX"; }
        case 0x3F: { return "ABX"; }

        case 0x40: { return "IMP"; }
        case 0x41: { return "IZX"; }
        case 0x42: { return "IMP"; }
        case 0x43: { return "IZX"; }
        case 0x44: { return "ZP0"; }
        case 0x45: { return "ZP0"; }
        case 0x46: { return "ZP0"; }
        case 0x47: { return "ZP0"; }
        case 0x48: { return "IMP"; }
        case 0x49: { return "IMM"; }
        case 0x4A: { return "IMP"; }
        case 0x4B: { return "IMP"; }
        case 0x4C: { return "ABS"; }
        case 0x4D: { return "ABS"; }
        case 0x4E: { return "ABS"; }
        case 0x4F: { return "ABS"; }

        case 0x50: { return "REL"; }
        case 0x51: { return "IZY"; }
        case 0x52: { return "IMP"; }
        case 0x53: { return "IZY"; }
        case 0x54: { return "ZPX"; }
        case 0x55: { return "ZPX"; }
        case 0x56: { return "ZPX"; }
        case 0x57: { return "ZPX"; }
        case 0x58: { return "IMP"; }
        case 0x59: { return "ABY"; }
        case 0x5A: { return "IMP"; }
        case 0x5B: { return "ABY"; }
        case 0x5C: { return "ABX"; }
        case 0x5D: { return "ABX"; }
        case 0x5E: { return "ABX"; }
        case 0x5F: { return "ABX"; }

        case 0x60: { return "IMP"; }
        case 0x61: { return "IZX"; }
        case 0x62: { return "IMP"; }
        case 0x63: { return "IZX"; }
        case 0x64: { return "ZP0"; }
        case 0x65: { return "ZP0"; }
        case 0x66: { return "ZP0"; }
        case 0x67: { return "ZP0"; }
        case 0x68: { return "IMP"; }
        case 0x69: { return "IMM"; }
        case 0x6A: { return "IMP"; }
        case 0x6B: { return "IMP"; }
        case 0x6C: { return "IND"; }
        case 0x6D: { return "ABS"; }
        case 0x6E: { return "ABS"; }
        case 0x6F: { return "ABS"; }

        case 0x70: { return "REL"; }
        case 0x71: { return "IZY"; }
        case 0x72: { return "IMP"; }
        case 0x73: { return "IZY"; }
        case 0x74: { return "ZPX"; }
        case 0x75: { return "ZPX"; }
        case 0x76: { return "ZPX"; }
        case 0x77: { return "ZPX"; }
        case 0x78: { return "IMP"; }
        case 0x79: { return "ABY"; }
        case 0x7A: { return "IMP"; }
        case 0x7B: { return "ABY"; }
        case 0x7C: { return "ABX"; }
        case 0x7D: { return "ABX"; }
        case 0x7E: { return "ABX"; }
        case 0x7F: { return "ABX"; }

        case 0x80: { return "IMM"; }
        case 0x81: { return "IZX"; }
        case 0x82: { return "IMP"; }
        case 0x83: { return "IZX"; }
        case 0x84: { return "ZP0"; }
        case 0x85: { return "ZP0"; }
        case 0x86: { return "ZP0"; }
        case 0x87: { return "ZP0"; }
        case 0x88: { return "IMP"; }
        case 0x89: { return "IMP"; }
        case 0x8A: { return "IMP"; }
        case 0x8B: { return "IMP"; }
        case 0x8C: { return "ABS"; }
        case 0x8D: { return "ABS"; }
        case 0x8E: { return "ABS"; }
        case 0x8F: { return "ABS"; }

        case 0x90: { return "REL"; }
        case 0x91: { return "IZY"; }
        case 0x92: { return "IMP"; }
        case 0x93: { return "IMP"; }
        case 0x94: { return "ZPX"; }
        case 0x95: { return "ZPX"; }
        case 0x96: { return "ZPY"; }
        case 0x97: { return "ZPY"; }
        case 0x98: { return "IMP"; }
        case 0x99: { return "ABY"; }
        case 0x9A: { return "IMP"; }
        case 0x9B: { return "IMP"; }
        case 0x9C: { return "IMP"; }
        case 0x9D: { return "ABX"; }
        case 0x9E: { return "IMP"; }
        case 0x9F: { return "IMP"; }

        case 0xA0: { return "IMM"; }
        case 0xA1: { return "IZX"; }
        case 0xA2: { return "IMM"; }
        case 0xA3: { return "IZX"; }
        case 0xA4: { return "ZP0"; }
        case 0xA5: { return "ZP0"; }
        case 0xA6: { return "ZP0"; }
        case 0xA7: { return "ZP0"; }
        case 0xA8: { return "IMP"; }
        case 0xA9: { return "IMM"; }
        case 0xAA: { return "IMP"; }
        case 0xAB: { return "IMP"; }
        case 0xAC: { return "ABS"; }
        case 0xAD: { return "ABS"; }
        case 0xAE: { return "ABS"; }
        case 0xAF: { return "ABS"; }

        case 0xB0: { return "REL"; }
        case 0xB1: { return "IZY"; }
        case 0xB2: { return "IMP"; }
        case 0xB3: { return "IZY"; }
        case 0xB4: { return "ZPX"; }
        case 0xB5: { return "ZPX"; }
        case 0xB6: { return "ZPY"; }
        case 0xB7: { return "ZPY"; }
        case 0xB8: { return "IMP"; }
        case 0xB9: { return "ABY"; }
        case 0xBA: { return "IMP"; }
        case 0xBB: { return "IMP"; }
        case 0xBC: { return "ABX"; }
        case 0xBD: { return "ABX"; }
        case 0xBE: { return "ABY"; }
        case 0xBF: { return "ABY"; }

        case 0xC0: { return "IMM"; }
        case 0xC1: { return "IZX"; }
        case 0xC2: { return "IMP"; }
        case 0xC3: { return "IZX"; }
        case 0xC4: { return "ZP0"; }
        case 0xC5: { return "ZP0"; }
        case 0xC6: { return "ZP0"; }
        case 0xC7: { return "ZP0"; }
        case 0xC8: { return "IMP"; }
        case 0xC9: { return "IMM"; }
        case 0xCA: { return "IMP"; }
        case 0xCB: { return "IMP"; }
        case 0xCC: { return "ABS"; }
        case 0xCD: { return "ABS"; }
        case 0xCE: { return "ABS"; }
        case 0xCF: { return "ABS"; }

        case 0xD0: { return "REL"; }
        case 0xD1: { return "IZY"; }
        case 0xD2: { return "IMP"; }
        case 0xD3: { return "IZY"; }
        case 0xD4: { return "ZPX"; }
        case 0xD5: { return "ZPX"; }
        case 0xD6: { return "ZPX"; }
        case 0xD7: { return "ZPX"; }
        case 0xD8: { return "IMP"; }
        case 0xD9: { return "ABY"; }
        case 0xDA: { return "IMP"; }
        case 0xDB: { return "ABY"; }
        case 0xDC: { return "ABX"; }
        case 0xDD: { return "ABX"; }
        case 0xDE: { return "ABX"; }
        case 0xDF: { return "ABX"; }

        case 0xE0: { return "IMM"; }
        case 0xE1: { return "IZX"; }
        case 0xE2: { return "IMP"; }
        case 0xE3: { return "IZX"; }
        case 0xE4: { return "ZP0"; }
        case 0xE5: { return "ZP0"; }
        case 0xE6: { return "ZP0"; }
        case 0xE7: { return "ZP0"; }
        case 0xE8: { return "IMP"; }
        case 0xE9: { return "IMM"; }
        case 0xEA: { return "IMP"; }
        case 0xEB: { return "IMM"; }
        case 0xEC: { return "ABS"; }
        case 0xED: { return "ABS"; }
        case 0xEE: { return "ABS"; }
        case 0xEF: { return "ABS"; }

        case 0xF0: { return "REL"; }
        case 0xF1: { return "IZY"; }
        case 0xF2: { return "IMP"; }
        case 0xF3: { return "IZY"; }
        case 0xF4: { return "ZPX"; }
        case 0xF5: { return "ZPX"; }
        case 0xF6: { return "ZPX"; }
        case 0xF7: { return "ZPX"; }
        case 0xF8: { return "IMP"; }
        case 0xF9: { return "ABY"; }
        case 0xFA: { return "IMP"; }
        case 0xFB: { return "ABY"; }
        case 0xFC: { return "ABX"; }
        case 0xFD: { return "ABX"; }
        case 0xFE: { return "ABX"; }
        case 0xFF: { return "ABX"; }
        default: { return "?*?"; }
    }
}

std::string instruction_name(uint8_t opcode) {
    switch (opcode) {
        case 0x00: { return "BRK"; }
        case 0x01: { return "ORA"; }
        case 0x02: { return "???"; }
        case 0x03: { return "SLO"; }
        case 0x04: { return "NOP"; }
        case 0x05: { return "ORA"; }
        case 0x06: { return "ASL"; }
        case 0x07: { return "SLO"; }
        case 0x08: { return "PHP"; }
        case 0x09: { return "ORA"; }
        case 0x0A: { return "ASL"; }
        case 0x0B: { return "???"; }
        case 0x0C: { return "NOP"; }
        case 0x0D: { return "ORA"; }
        case 0x0E: { return "ASL"; }
        case 0x0F: { return "SLO"; }

        case 0x10: { return "BPL"; }
        case 0x11: { return "ORA"; }
        case 0x12: { return "???"; }
        case 0x13: { return "SLO"; }
        case 0x14: { return "NOP"; }
        case 0x15: { return "ORA"; }
        case 0x16: { return "ASL"; }
        case 0x17: { return "SLO"; }
        case 0x18: { return "CLC"; }
        case 0x19: { return "ORA"; }
        case 0x1A: { return "NOP"; }
        case 0x1B: { return "SLO"; }
        case 0x1C: { return "NOP"; }
        case 0x1D: { return "ORA"; }
        case 0x1E: { return "ASL"; }
        case 0x1F: { return "SLO"; }

        case 0x20: { return "JSR"; }
        case 0x21: { return "AND"; }
        case 0x22: { return "???"; }
        case 0x23: { return "RLA"; }
        case 0x24: { return "BIT"; }
        case 0x25: { return "AND"; }
        case 0x26: { return "ROL"; }
        case 0x27: { return "RLA"; }
        case 0x28: { return "PLP"; }
        case 0x29: { return "AND"; }
        case 0x2A: { return "ROL"; }
        case 0x2B: { return "ANC"; }
        case 0x2C: { return "BIT"; }
        case 0x2D: { return "AND"; }
        case 0x2E: { return "ROL"; }
        case 0x2F: { return "RLA"; }

        case 0x30: { return "BMI"; }
        case 0x31: { return "AND"; }
        case 0x32: { return "???"; }
        case 0x33: { return "RLA"; }
        case 0x34: { return "NOP"; }
        case 0x35: { return "AND"; }
        case 0x36: { return "ROL"; }
        case 0x37: { return "RLA"; }
        case 0x38: { return "SEC"; }
        case 0x39: { return "AND"; }
        case 0x3A: { return "NOP"; }
        case 0x3B: { return "RLA"; }
        case 0x3C: { return "NOP"; }
        case 0x3D: { return "AND"; }
        case 0x3E: { return "ROL"; }
        case 0x3F: { return "RLA"; }

        case 0x40: { return "RTI"; }
        case 0x41: { return "EOR"; }
        case 0x42: { return "???"; }
        case 0x43: { return "SRE"; }
        case 0x44: { return "NOP"; }
        case 0x45: { return "EOR"; }
        case 0x46: { return "LSR"; }
        case 0x47: { return "SRE"; }
        case 0x48: { return "PHA"; }
        case 0x49: { return "EOR"; }
        case 0x4A: { return "LSR"; }
        case 0x4B: { return "???"; }
        case 0x4C: { return "JMP"; }
        case 0x4D: { return "EOR"; }
        case 0x4E: { return "LSR"; }
        case 0x4F: { return "SRE"; }

        case 0x50: { return "BVC"; }
        case 0x51: { return "EOR"; }
        case 0x52: { return "???"; }
        case 0x53: { return "SRE"; }
        case 0x54: { return "NOP"; }
        case 0x55: { return "EOR"; }
        case 0x56: { return "LSR"; }
        case 0x57: { return "SRE"; }
        case 0x58: { return "CLI"; }
        case 0x59: { return "EOR"; }
        case 0x5A: { return "NOP"; }
        case 0x5B: { return "SRE"; }
        case 0x5C: { return "NOP"; }
        case 0x5D: { return "EOR"; }
        case 0x5E: { return "LSR"; }
        case 0x5F: { return "SRE"; }

        case 0x60: { return "RTS"; }
        case 0x61: { return "ADC"; }
        case 0x62: { return "???"; }
        case 0x63: { return "RRA"; }
        case 0x64: { return "NOP"; }
        case 0x65: { return "ADC"; }
        case 0x66: { return "ROR"; }
        case 0x67: { return "RRA"; }
        case 0x68: { return "PLA"; }
        case 0x69: { return "ADC"; }
        case 0x6A: { return "ROR"; }
        case 0x6B: { return "???"; }
        case 0x6C: { return "JMP"; }
        case 0x6D: { return "ADC"; }
        case 0x6E: { return "ROR"; }
        case 0x6F: { return "RRA"; }

        case 0x70: { return "BVS"; }
        case 0x71: { return "ADC"; }
        case 0x72: { return "???"; }
        case 0x73: { return "RRA"; }
        case 0x74: { return "NOP"; }
        case 0x75: { return "ADC"; }
        case 0x76: { return "ROR"; }
        case 0x77: { return "RRA"; }
        case 0x78: { return "SEI"; }
        case 0x79: { return "ADC"; }
        case 0x7A: { return "NOP"; }
        case 0x7B: { return "RRA"; }
        case 0x7C: { return "NOP"; }
        case 0x7D: { return "ADC"; }
        case 0x7E: { return "ROR"; }
        case 0x7F: { return "RRA"; }

        case 0x80: { return "NOP"; }
        case 0x81: { return "STA"; }
        case 0x82: { return "???"; }
        case 0x83: { return "SAX"; }
        case 0x84: { return "STY"; }
        case 0x85: { return "STA"; }
        case 0x86: { return "STX"; }
        case 0x87: { return "SAX"; }
        case 0x88: { return "DEY"; }
        case 0x89: { return "???"; }
        case 0x8A: { return "TXA"; }
        case 0x8B: { return "???"; }
        case 0x8C: { return "STY"; }
        case 0x8D: { return "STA"; }
        case 0x8E: { return "STX"; }
        case 0x8F: { return "SAX"; }

        case 0x90: { return "BCC"; }
        case 0x91: { return "STA"; }
        case 0x92: { return "???"; }
        case 0x93: { return "???"; }
        case 0x94: { return "STY"; }
        case 0x95: { return "STA"; }
        case 0x96: { return "STX"; }
        case 0x97: { return "SAX"; }
        case 0x98: { return "TYA"; }
        case 0x99: { return "STA"; }
        case 0x9A: { return "TXS"; }
        case 0x9B: { return "???"; }
        case 0x9C: { return "???"; }
        case 0x9D: { return "STA"; }
        case 0x9E: { return "???"; }
        case 0x9F: { return "???"; }

        case 0xA0: { return "LDY"; }
        case 0xA1: { return "LDA"; }
        case 0xA2: { return "LDX"; }
        case 0xA3: { return "LAX"; }
        case 0xA4: { return "LDY"; }
        case 0xA5: { return "LDA"; }
        case 0xA6: { return "LDX"; }
        case 0xA7: { return "LAX"; }
        case 0xA8: { return "TAY"; }
        case 0xA9: { return "LDA"; }
        case 0xAA: { return "TAX"; }
        case 0xAB: { return "???"; }
        case 0xAC: { return "LDY"; }
        case 0xAD: { return "LDA"; }
        case 0xAE: { return "LDX"; }
        case 0xAF: { return "LAX"; }

        case 0xB0: { return "BCS"; }
        case 0xB1: { return "LDA"; }
        case 0xB2: { return "???"; }
        case 0xB3: { return "LAX"; }
        case 0xB4: { return "LDY"; }
        case 0xB5: { return "LDA"; }
        case 0xB6: { return "LDX"; }
        case 0xB7: { return "LAX"; }
        case 0xB8: { return "CLV"; }
        case 0xB9: { return "LDA"; }
        case 0xBA: { return "TSX"; }
        case 0xBB: { return "???"; }
        case 0xBC: { return "LDY"; }
        case 0xBD: { return "LDA"; }
        case 0xBE: { return "LDX"; }
        case 0xBF: { return "LAX"; }

        case 0xC0: { return "CPY"; }
        case 0xC1: { return "CMP"; }
        case 0xC2: { return "???"; }
        case 0xC3: { return "DCP"; }
        case 0xC4: { return "CPY"; }
        case 0xC5: { return "CMP"; }
        case 0xC6: { return "DEC"; }
        case 0xC7: { return "DCP"; }
        case 0xC8: { return "INY"; }
        case 0xC9: { return "CMP"; }
        case 0xCA: { return "DEX"; }
        case 0xCB: { return "???"; }
        case 0xCC: { return "CPY"; }
        case 0xCD: { return "CMP"; }
        case 0xCE: { return "DEC"; }
        case 0xCF: { return "DCP"; }

        case 0xD0: { return "BNE"; }
        case 0xD1: { return "CMP"; }
        case 0xD2: { return "???"; }
        case 0xD3: { return "DCP"; }
        case 0xD4: { return "NOP"; }
        case 0xD5: { return "CMP"; }
        case 0xD6: { return "DEC"; }
        case 0xD7: { return "DCP"; }
        case 0xD8: { return "CLD"; }
        case 0xD9: { return "CMP"; }
        case 0xDA: { return "NOP"; }
        case 0xDB: { return "DCP"; }
        case 0xDC: { return "NOP"; }
        case 0xDD: { return "CMP"; }
        case 0xDE: { return "DEC"; }
        case 0xDF: { return "DCP"; }

        case 0xE0: { return "CPX"; }
        case 0xE1: { return "SBC"; }
        case 0xE2: { return "???"; }
        case 0xE3: { return "ISB"; }
        case 0xE4: { return "CPX"; }
        case 0xE5: { return "SBC"; }
        case 0xE6: { return "INC"; }
        case 0xE7: { return "ISB"; }
        case 0xE8: { return "INX"; }
        case 0xE9: { return "SBC"; }
        case 0xEA: { return "NOP"; }
        case 0xEB: { return "SBC"; }
        case 0xEC: { return "CPX"; }
        case 0xED: { return "SBC"; }
        case 0xEE: { return "INC"; }
        case 0xEF: { return "ISB"; }

        case 0xF0: { return "BEQ"; }
        case 0xF1: { return "SBC"; }
        case 0xF2: { return "???"; }
        case 0xF3: { return "ISB"; }
        case 0xF4: { return "NOP"; }
        case 0xF5: { return "SBC"; }
        case 0xF6: { return "INC"; }
        case 0xF7: { return "ISB"; }
        case 0xF8: { return "SED"; }
        case 0xF9: { return "SBC"; }
        case 0xFA: { return "NOP"; }
        case 0xFB: { return "ISB"; }
        case 0xFC: { return "NOP"; }
        case 0xFD: { return "SBC"; }
        case 0xFE: { return "INC"; }
        case 0xFF: { return "ISB"; }
        default: { return "?@?"; }
    }
}
