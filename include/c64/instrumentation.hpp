#ifndef C64_INSTRUMENTATION_HPP
#define C64_INSTRUMENTATION_HPP


#include <string>
#include "cpu_6502.hpp"

namespace c64tools {
    std::string address(CPU6502 const& cpu, CPUIO& bus);

    std::string cpu_flags_to_string(uint8_t flags);
}


#endif //C64_INSTRUMENTATION_HPP
