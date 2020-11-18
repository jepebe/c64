#include "catch2.hpp"
#include "common.hpp"
#include <c64/cpu_6502.hpp>

#include <fmt/format.h>
#include <chrono>
#include <c64/instrumentation.hpp>

// I believe the timingtest.data has some errors e.x. STA at $10C4 is 6 instead of 5 ?

TEST_CASE("Run Timing Test") {
    SECTION("Timing test") {
        auto cpu = CPU6502();
        auto data = load_rom_file("roms/timingtest.bin");
        auto bus = MockBus();
        for (int index = 0; index < data.size(); index++) {
            bus.write(0x1000 + index, data[index]);
        }

        // Start vector for the test
        bus.write(0xFFFC, 0x00);
        bus.write(0xFFFD, 0x10);
        cpu.reset(bus);
        cpu.cycles = 0;

        auto start = std::chrono::high_resolution_clock::now();

        int count = 0;
        while (true) {
            auto pc = cpu.pc;
            count++;
            while (!cpu.complete()) {
                cpu.clock(bus);
            }

            auto opcode = bus.read(cpu.pc, true);
            auto info = cpu.instruction_info(opcode);
            auto addr = c64tools::address(cpu, bus);
            fmt::print("${:04X} {} {:<7} {} cyc\n", pc, info.instruction, addr, cpu.clock_count);
            if (cpu.pc == 0x1269) {
                break;
            }
            cpu.clock(bus);
        }
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        fmt::print("Total number of instructions: {:d}\n", count);
        fmt::print("Running time: {:d} ms.\n", duration.count());
        REQUIRE(cpu.clock_count == 1141);
    }

}