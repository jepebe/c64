#include "catch2.hpp"
#include "common.hpp"
#include <c64/cpu_6502.hpp>

#include <fmt/format.h>
#include <chrono>
#include "tools.hpp"

//#define private public  // Hack to expose private data - so the implementation don't

void run_klaus_test(CPU6502 &cpu, const std::string &rom_path, uint16_t stop_pc, bool trace) {
    auto data = load_rom_file(rom_path);
    auto bus = MockBus();
    for (int index = 0; index < data.size(); index++) {
        bus.write(index, data[index]);
    }

    // Start vector for the test
    bus.write(0xFFFC, 0x00);
    bus.write(0xFFFD, 0x04);
    cpu.reset(bus);

    auto start = std::chrono::high_resolution_clock::now();

    uint16_t pc = -1;
    int count = 0;
    while (cpu.pc != stop_pc) {
        count++;
        while (!cpu.complete()) {
            cpu.clock(bus);
        }

        if(trace) {
            std::string status = nestools::full_cpu_status_as_string(cpu, bus);
            fmt::print("{:d} -> {}\n", count, status);
        }
        cpu.clock(bus);

        if (pc == cpu.pc) {
            std::string status = nestools::full_cpu_status_as_string(cpu, bus);
            fmt::print("{:d} -> {}\n", count, status);
            REQUIRE_MESSAGE(pc != cpu.pc,
                            fmt::format("Trapped at ${:04X} after {} instructions",
                                        cpu.pc, count));
        }

        pc = cpu.pc;
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    fmt::print("Total number of instructions: {:d}\n", count);
    fmt::print("Running time: {:d} ms.\n", duration.count());
}

TEST_CASE("Run Klaus' Tests") {
    SECTION("Klaus Functional Test") {
        // Decimal testing starts at 0x336D
        // Complete testing with BCD ends at 0x3469
        auto cpu = CPU6502();
        run_klaus_test(cpu, "roms/6502_functional_test.bin", 0x3469, false);
    }

    SECTION("Common functionality test") {
        auto data = load_rom_file("roms/6502_functional_test.bin");
        REQUIRE(data.size() == 0x10000);

        auto bus = MockBus();
        for (int index = 0; index < data.size(); index++) {
            auto value = data[index];
            bus.write(index, value);
        }

        REQUIRE(data[0] == bus.read(0, true));
        REQUIRE(data[0] != bus.read(0x00FF, true));
        REQUIRE(data[0x7FFF] == bus.read(0x7FFF, true));
        REQUIRE(data[0xFFFF] == bus.read(0xFFFF, true));
    }
}