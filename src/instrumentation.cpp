#include <fmt/format.h>
#include "c64/instrumentation.hpp"
#include "c64/cpu_6502.hpp"


std::string c64tools::cpu_flags_to_string(uint8_t flags) {
    auto c = is_flag_set(Flags6502::C, flags) ? "C" : ".";
    auto z = is_flag_set(Flags6502::Z, flags) ? "Z" : ".";
    auto i = is_flag_set(Flags6502::I, flags) ? "I" : ".";
    auto d = is_flag_set(Flags6502::D, flags) ? "D" : ".";
    auto b = is_flag_set(Flags6502::B, flags) ? "B" : ".";
    auto u = is_flag_set(Flags6502::U, flags) ? "U" : ".";
    auto v = is_flag_set(Flags6502::V, flags) ? "V" : ".";
    auto n = is_flag_set(Flags6502::N, flags) ? "N" : ".";
    return fmt::format("{}{}{}{}{}{}{}{}", c, z, i, d, b, u, v, n);
}

constexpr unsigned int hash(const char* str, int h = 0) {
    return !str[h] ? 5381 : (hash(str, h + 1) * 33) ^ str[h];
}

std::string c64tools::address(CPU6502 const& cpu, CPUIO& bus) {
    auto opcode = bus.read(cpu.pc, true);
    uint16_t addr = cpu.pc + 1;
    auto instruction_info = cpu.instruction_info(opcode);
    auto addr_mode_name = instruction_info.addr_mode;
    auto instruction_name = instruction_info.instruction;

    switch (hash(addr_mode_name)) {
        case hash("IMM"): {
            uint8_t m = bus.read(addr, true);
            return fmt::format("#${:02X}", m);
        }
        case hash("IMP"): {
            switch (hash(instruction_name)) {
                case hash("LSR"):
                case hash("ASL"):
                case hash("ROR"):
                case hash("ROL"): {
                    return fmt::format("A");
                }
                default: {
                    return fmt::format("");
                }
            }
        }
        case hash("REL"): {
            int8_t rel = bus.read(addr, true);
            addr = addr + 1 + rel;
            return fmt::format("${:04X}", addr);
        }
        case hash("ABS"): {
            auto lo = bus.read(addr, true);
            auto hi = bus.read(addr + 1, true);
            addr = (hi << 8u) | lo;
            return fmt::format("${:04X}", addr);
        }
        case hash("ABX"): {
            uint8_t lo = bus.read(addr, true);
            uint8_t hi = bus.read(addr + 1, true);
            uint16_t origin = (hi << 8u) | lo;
            return fmt::format("${:04X},X", origin);
        }
        case hash("ABY"): {
            uint8_t lo = bus.read(addr, true);
            uint8_t hi = bus.read(addr + 1, true);
            uint16_t origin = (hi << 8u) | lo;
            return fmt::format("${:04X},Y", origin);
        }
        case hash("IND"): {
            uint8_t lo = bus.read(addr, true);
            uint8_t hi = bus.read(addr + 1, true);
            uint16_t ptr = (hi << 8u) | lo;

            return fmt::format("(${:04X})", ptr);
        }
        case hash("IZX"): {
            uint8_t ptr = bus.read(addr, true);
            return fmt::format("(${:02X},X)", ptr);
        }
        case hash("IZY"): {
            uint8_t ptr = bus.read(addr, true);
            return fmt::format("(${:02X}),Y", ptr);
        }
        case hash("ZP0"): {
            auto lo = bus.read(addr, true);
            addr = lo;
            return fmt::format("${:02X}", addr);
        }
        case hash("ZPX"): {
            uint8_t offset = bus.read(addr, true);
            return fmt::format("${:02X},X", offset);
        }
        case hash("ZPY"): {
            uint8_t offset = bus.read(addr, true);

            return fmt::format("${:02X},Y", offset);
        }
        default: {
            return fmt::format("Unknown {}", addr_mode_name);
        }
    }
}