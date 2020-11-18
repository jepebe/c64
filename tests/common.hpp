#ifndef C64_COMMON_HPP
#define C64_COMMON_HPP

#include <fstream>
#include <vector>
#include <c64/cpu_6502.hpp>

#define CHECK_MESSAGE(cond, msg) do { INFO(msg); CHECK(cond); } while((void)0, 0)
#define REQUIRE_MESSAGE(cond, msg) do { INFO(msg); REQUIRE(cond); } while((void)0, 0)

class MockBus : public CPUIO {

    uint8_t ram[0x10000] = {};

public:
    void write(uint16_t addr, uint8_t value) override {
        ram[addr] = value;
    }

    uint8_t read(uint16_t addr, bool read_only) override {
        return ram[addr];
    }

    void interrupt(Interrupt interrupt) override {

    }
};

std::vector<std::uint8_t> load_rom_file(std::string const &filepath);

#endif //C64_COMMON_HPP
