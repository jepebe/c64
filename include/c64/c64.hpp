#ifndef NES_C64_HPP
#define NES_C64_HPP


#include "cpu_6502.hpp"

class C64 : public CPUIO {
private:
    CPU6502 cpu;
    uint8_t ram[0x10000];
    uint8_t basic_rom[0x2000];
    uint8_t char_rom[0x1000];
    uint8_t kernal_rom[0x2000];

    uint64_t system_clock = 0;
    Interrupt interrupt_state = Interrupt::None;

public:
    C64();

    void write(uint16_t addr, uint8_t value) override;

    uint8_t read(uint16_t addr, bool read_only) override;

    void reset();

    bool clock();

    void interrupt(Interrupt interrupt) override;

    [[nodiscard]] CPU6502 const& get_cpu() const;


};

#endif //NES_C64_HPP
