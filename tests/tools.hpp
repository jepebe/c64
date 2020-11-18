#ifndef NES_TOOLS_HPP
#define NES_TOOLS_HPP

#include "c64/cpu_6502.hpp"
#include <string>
#include <map>

namespace nestools {
    std::string full_cpu_status_as_string(const CPU6502 &cpu, CPUIO &bus);

    std::string cpu_status_summary(const CPU6502 &cpu, CPUIO &bus);

    std::map<uint16_t, std::string> disassemble(const CPU6502 &cpu, CPUIO &bus, uint16_t from, uint16_t to);
}

#endif //NES_TOOLS_HPP
