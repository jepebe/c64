#include "tools.hpp"
#include <fmt/format.h>

constexpr unsigned int hash(const char *str, int h = 0) {
    return !str[h] ? 5381 : (hash(str, h + 1u) * 33u) ^ str[h];
}

static int instruction_size(const CPU6502 &cpu, CPUIO &bus) {
    auto opcode = bus.read(cpu.pc, true);

    auto addr_mode = cpu.instruction_info(opcode).addr_mode;

    switch (hash(addr_mode)) {
        case hash("IMP"): {
            return 1;
        }
        case hash("IMM"):
        case hash("ZP0"):
        case hash("ZPX"):
        case hash("ZPY"):
        case hash("REL"):
        case hash("IZX"):
        case hash("IZY"):
        case hash("IZ0"): {
            return 2;
        }
        case hash("ABS"):
        case hash("ABX"):
        case hash("ABY"):
        case hash("IND"):
        case hash("ZRL"): {
            return 3;
        }

        default:
            return -1;
    }
}

static std::string relevant_bytes(const CPU6502 &cpu, CPUIO &bus, const char * addr_mode) {
    auto opc = bus.read(cpu.pc, true);
    switch (hash(addr_mode)) {
        case hash("IMP"): {
            return fmt::format("{:02X}", opc);
        }
        case hash("IMM"):
        case hash("ZP0"):
        case hash("ZPX"):
        case hash("ZPY"):
        case hash("REL"):
        case hash("IZX"):
        case hash("IZY"):
        case hash("IZ0"): {
            auto value = bus.read(cpu.pc + 1, true);
            return fmt::format("{:02X} {:02X}", opc, value);
        }
        case hash("ABS"):
        case hash("ABX"):
        case hash("ABY"):
        case hash("IND"):
        case hash("IAX"):
        case hash("ZRL"): {
            auto a = bus.read(cpu.pc + 1, true);
            auto b = bus.read(cpu.pc + 2, true);
            return fmt::format("{:02X} {:02X} {:02X}", opc, a, b);
        }

        default:
            return fmt::format("Unknown {}", addr_mode);
    }
}

static std::string flags_to_string(uint8_t flags) {
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

static std::string address(const CPU6502 &cpu, CPUIO &bus) {
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
            switch (hash(instruction_name)) {
                case hash("JMP"):
                case hash("JSR"): {
                    return fmt::format("${:04X}", addr);
                }
                default: {
                    auto value = bus.read(addr, true);
                    return fmt::format("${:04X} = {:02X}", addr, value);
                }
            }
        }
        case hash("ABX"): {
            uint8_t lo = bus.read(addr, true);
            uint8_t hi = bus.read(addr + 1, true);
            uint16_t origin = (hi << 8u) | lo;
            addr = origin + cpu.x;
            uint8_t value = bus.read(addr, true);
            return fmt::format("${:04X},X @ {:04X} = {:02X}", origin, addr, value);
        }
        case hash("ABY"): {
            uint8_t lo = bus.read(addr, true);
            uint8_t hi = bus.read(addr + 1, true);
            uint16_t origin = (hi << 8u) | lo;
            addr = origin + cpu.y;
            uint8_t value = bus.read(addr, true);
            return fmt::format("${:04X},Y @ {:04X} = {:02X}", origin, addr, value);
        }
        case hash("IND"): {
            uint8_t lo = bus.read(addr, true);
            uint8_t hi = bus.read(addr + 1, true);
            uint16_t ptr = (hi << 8u) | lo;
            //todo: does not apply to 65c02
            if (lo == 0x00FF) {
                hi = bus.read(ptr & 0xFF00u, true);
            } else {
                hi = bus.read(ptr + 1, true);
            }
            lo = bus.read(ptr, true);

            addr = (hi << 8u) | lo;
            return fmt::format("(${:04X}) = {:04X}", ptr, addr);
        }
        case hash("IAX"): {
            auto lo = bus.read(addr, true);
            auto hi = bus.read(addr + 1, true);
            auto origin = ((hi << 8u) | lo);
            auto ptr_addr = origin + cpu.x;
            lo = bus.read(ptr_addr, true);
            hi = bus.read(ptr_addr + 1, true);
            addr = ((hi << 8u) | lo);
            return fmt::format("(${:04X},X) @ ${:04X}-> ${:04X}", origin, ptr_addr, addr);
        }
        case hash("IZ0"): {
            uint8_t zp = bus.read(addr, true);
            auto lo = bus.read((zp) & 0x00FFu, true);
            auto hi = bus.read((zp + 1u) & 0x00FFu, true);
            addr = (hi << 8u) | lo;
            auto value = bus.read(addr, true);

            return fmt::format("(${:02X}) @ {:04X} = {:02X}", zp, addr, value);
        }
        case hash("IZX"): {
            uint8_t ptr = bus.read(addr, true);
            auto lo = bus.read((ptr + cpu.x) & 0x00FFu, true);
            auto hi = bus.read((ptr + cpu.x + 1u) & 0x00FFu, true);
            addr = (hi << 8u) | lo;
            uint8_t value = bus.read(addr, true);
            uint16_t ptrx = (ptr + cpu.x) & 0x00FFu;
            return fmt::format("(${:02X},X) @ {:02X} = {:04X} = {:02X}", ptr, ptrx, addr, value);
        }
        case hash("IZY"): {
            uint8_t ptr = bus.read(addr, true);
            auto lo = bus.read(ptr & 0x00FFu, true);
            auto hi = bus.read((ptr + 1u) & 0x00FFu, true);
            uint16_t ptry = (hi << 8u) | lo;
            addr = ptry + cpu.y;
            uint8_t value = bus.read(addr, true);
            return fmt::format("(${:02X}),Y = {:04X} @ {:04X} = {:02X}", ptr, ptry, addr, value);
        }
        case hash("ZP0"): {
            auto lo = bus.read(addr, true);
            addr = lo;
            uint8_t value = bus.read(addr, true);
            return fmt::format("${:02X} = {:02X}", addr, value);
        }
        case hash("ZPX"): {
            uint8_t offset = bus.read(addr, true);
            addr = (offset + cpu.x) & 0x00FFu;
            uint8_t value = bus.read(addr, true);
            return fmt::format("${:02X},X @ {:02X} = {:02X}", offset, addr, value);
        }
        case hash("ZPY"): {
            uint8_t offset = bus.read(addr, true);
            addr = (offset + cpu.y) & 0x00FFu;
            uint8_t value = bus.read(addr, true);
            return fmt::format("${:02X},Y @ {:02X} = {:02X}", offset, addr, value);
        }
        case hash("ZRL"): {
            uint8_t zp_addr = bus.read(addr, true);
            uint8_t value = bus.read(zp_addr, true);
            int8_t rel = bus.read(addr + 1, true);
            addr += 2 + rel;

            return fmt::format("${:02X} = {:02X} -> ${:04X}", zp_addr, value, addr);
        }
        default: {
            return fmt::format("Unknown {}", addr_mode_name);
        }
    }
}

std::string nestools::full_cpu_status_as_string(const CPU6502 &cpu, CPUIO &bus) {
    auto pc = cpu.pc;
    auto opcode = bus.read(pc, true);

    auto instruction_info = cpu.instruction_info(opcode);
    auto addr_mode = instruction_info.addr_mode;
    auto bytes = relevant_bytes(cpu, bus, addr_mode);
    auto ns = instruction_info.non_standard ? "*" : " ";
    auto name = instruction_info.instruction;
    auto flags = flags_to_string(cpu.status);
    auto addressing = address(cpu, bus);

    return fmt::format(
            "{:04X}  {:<9}{}{} {:<28}A:{:02X} X:{:02X} Y:{:02X} P:{:02X} SP:{:02X} CYC:{} {} {}",
            pc, bytes, ns, name, addressing, cpu.a, cpu.x, cpu.y, cpu.status, cpu.stkp,
            cpu.clock_count, instruction_info.addr_mode, flags_to_string(cpu.status));
}

static std::string summary_cpu_status(const CPU6502 &cpu, CPUIO &bus) {
    auto pc = cpu.pc;
    auto opcode = bus.read(pc, true);

    auto instruction_info = cpu.instruction_info(opcode);
    auto addr_mode = instruction_info.addr_mode;
    auto ns = instruction_info.non_standard ? "*" : " ";
    auto name = instruction_info.instruction;

    auto addressing = address(cpu, bus);
    return fmt::format("{:04X} {}{} {:<18} [{}]", pc, ns, name, addressing, addr_mode);
}

std::string nestools::cpu_status_summary(const CPU6502 & cpu, CPUIO & bus) {
    return summary_cpu_status(cpu, bus);
}

std::map<uint16_t, std::string> nestools::disassemble(const CPU6502 &cpu, CPUIO &bus, uint16_t from, uint16_t to) {
    std::map<uint16_t, std::string> map;
    uint32_t addr = from;

    while (addr <= to) {
//        cpu.pc = addr;
        auto size = instruction_size(cpu, bus);
        map[addr] = summary_cpu_status(cpu, bus);
        addr += size;
    }

    return map;
}
