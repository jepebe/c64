#ifndef C64_PYC64_HPP
#define C64_PYC64_HPP


#include "c64.hpp"

class pyC64 {
    C64 c64;

public:
    pyC64();
    bool clock();
    void reset();
    [[nodiscard]] const CPU6502 & cpu() const;
    std::string disassemble(uint16_t addr);
};


#endif //C64_PYC64_HPP
