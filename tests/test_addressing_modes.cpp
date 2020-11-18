#include "catch2.hpp"

#define protected public
#include "c64/cpu_6502.hpp"
#include "common.hpp"

TEST_CASE("Addressing modes") {
    auto cpu = CPU6502();
    auto bus = MockBus();
    SECTION("IMP") {
        REQUIRE(!cpu.implied_has_value);
        cpu.a = 0xA5;
        REQUIRE(cpu.IMP(bus) == 0);
        REQUIRE(cpu.implied == 0xA5);
    }

    SECTION("IMM") {
        bus.write(0x0000, 0xA5);
        REQUIRE(cpu.pc == 0x0000);
        REQUIRE(cpu.IMM(bus) == 0);
        REQUIRE(cpu.pc == 0x0001);
        REQUIRE(cpu.fetch(bus) == 0xA5);
    }

    SECTION("ZP0") {
        bus.write(0x007F, 0xA5);
        bus.write(0x7FA5, 0x7F);
        cpu.pc = 0x7FA5;
        REQUIRE(cpu.ZP0(bus) == 0);
        REQUIRE(cpu.addr_abs == 0x007F);
        REQUIRE(cpu.fetch(bus) == 0xA5);
        REQUIRE(cpu.pc == 0x7FA6);
    }

    SECTION("ZPX") {
        bus.write(0x007F, 0xA5);
        bus.write(0x7FA5, 0x7F);
        bus.write(0x007F + 0x0A, 0xFF);
        cpu.pc = 0x7FA5;
        cpu.x = 0x0A;
        REQUIRE(cpu.ZPX(bus) == 0);
        REQUIRE(cpu.addr_abs == 0x007F + 0x0A);
        REQUIRE(cpu.fetch(bus) == 0xFF);
        REQUIRE(cpu.pc == 0x7FA6);

        bus.write(0x7FA5, 0xFF);
        bus.write(0x0004, 0xDE);
        cpu.pc = 0x7FA5;
        cpu.x = 0x05;
        REQUIRE(cpu.ZPX(bus) == 0);
        REQUIRE(cpu.addr_abs == 0x0004);
        REQUIRE(cpu.fetch(bus) == 0xDE);
        REQUIRE(cpu.pc == 0x7FA6);
    }

    SECTION("ZPY") {
        bus.write(0x007F, 0xA5);
        bus.write(0x7FA5, 0x7F);
        bus.write(0x007F + 0x0A, 0xFF);
        cpu.pc = 0x7FA5;
        cpu.y = 0x0A;
        REQUIRE(cpu.ZPY(bus) == 0);
        REQUIRE(cpu.addr_abs == 0x007F + 0x0A);
        REQUIRE(cpu.fetch(bus) == 0xFF);
        REQUIRE(cpu.pc == 0x7FA6);

        bus.write(0x7FA5, 0xFF);
        bus.write(0x0004, 0xDE);
        cpu.pc = 0x7FA5;
        cpu.y = 0x05;
        REQUIRE(cpu.ZPY(bus) == 0);
        REQUIRE(cpu.addr_abs == 0x0004);
        REQUIRE(cpu.fetch(bus) == 0xDE);
        REQUIRE(cpu.pc == 0x7FA6);
    }

    SECTION("REL") {
        bus.write(0x7FA5, 0x07);
        cpu.pc = 0x7FA5;
        REQUIRE(cpu.REL(bus) == 0);
        REQUIRE(cpu.addr_rel == 0x07);
        REQUIRE(cpu.pc == 0x7FA6);

        bus.write(0x7FA5, 0xF7);
        cpu.pc = 0x7FA5;
        REQUIRE(cpu.REL(bus) == 0);
        REQUIRE(cpu.addr_rel == 0xF7);
        REQUIRE(cpu.pc == 0x7FA6);
    }

    SECTION("ABS") {
        bus.write(0x7FA5, 0xAD);
        bus.write(0x7FA6, 0xDE);
        cpu.pc = 0x7FA5;
        REQUIRE(cpu.ABS(bus) == 0);
        REQUIRE(cpu.addr_abs == 0xDEAD);
        REQUIRE(cpu.pc == 0x7FA7);
    }


    SECTION("ABX") {
        bus.write(0x7FA5, 0xAD);
        bus.write(0x7FA6, 0xDE);
        cpu.pc = 0x7FA5;
        cpu.x = 0x0A;
        REQUIRE(cpu.ABX(bus) == 0);
        REQUIRE(cpu.addr_abs == 0xDEAD + 0x0A);
        REQUIRE(cpu.pc == 0x7FA7);

        cpu.pc = 0x7FA5;
        cpu.x = 0xFF;
        REQUIRE(cpu.ABX(bus) == 1);
        REQUIRE(cpu.addr_abs == 0xDEAD + 0xFF);
        REQUIRE(cpu.pc == 0x7FA7);
    }

    SECTION("ABY") {
        bus.write(0x7FA5, 0xAD);
        bus.write(0x7FA6, 0xDE);
        cpu.pc = 0x7FA5;
        cpu.y = 0x0A;
        REQUIRE(cpu.ABY(bus) == 0);
        REQUIRE(cpu.addr_abs == 0xDEAD + 0x0A);
        REQUIRE(cpu.pc == 0x7FA7);

        cpu.pc = 0x7FA5;
        cpu.y = 0xFF;
        REQUIRE(cpu.ABY(bus) == 1);
        REQUIRE(cpu.addr_abs == 0xDEAD + 0xFF);
        REQUIRE(cpu.pc == 0x7FA7);
    }

    SECTION("IND") {
        bus.write(0x7FA5, 0xAD);
        bus.write(0x7FA6, 0xDE);
        bus.write(0xDEAD, 0x0B);
        bus.write(0xDEAD + 1, 0xB0);
        cpu.pc = 0x7FA5;
        REQUIRE(cpu.IND(bus) == 0);
        REQUIRE(cpu.addr_abs == 0xB00B);
        REQUIRE(cpu.pc == 0x7FA7);

        // This tests the hardware bug where IND goes to $A500 instead of $A600
        bus.write(0x7FA7, 0xFF);
        bus.write(0x7FA8, 0xA5);
        bus.write(0xA5FF, 0xC1);
        bus.write(0xA600, 0xC2);
        bus.write(0xA500, 0xC3);

        cpu.pc = 0x7FA7;
        REQUIRE(cpu.IND(bus) == 0);
        REQUIRE(cpu.addr_abs == 0xC3C1);
        REQUIRE(cpu.pc == 0x7FA9);
    }

    SECTION("IZX") {
        bus.write(0x7FA5, 0xA8);
        bus.write(0x00AD, 0x0B);
        bus.write(0x00AD + 1, 0xB0);
        cpu.pc = 0x7FA5;
        cpu.x = 0x05;
        REQUIRE(cpu.IZX(bus) == 0);
        REQUIRE(cpu.addr_abs == 0xB00B);
        REQUIRE(cpu.pc == 0x7FA6);

        bus.write(0x7FA5, 0xFA);
        bus.write(0x00FF, 0x0B);
        bus.write(0x0000, 0xB0);
        bus.write(0x0100, 0xC0);

        cpu.pc = 0x7FA5;
        REQUIRE(cpu.IZX(bus) == 0);
        REQUIRE(cpu.addr_abs == 0xB00B);
        REQUIRE(cpu.pc == 0x7FA6);
    }

    SECTION("IZY") {
        bus.write(0x7FA5, 0xAD);

        bus.write(0x00AD, 0x0B);
        bus.write(0x00AD + 1, 0xB0);
        cpu.pc = 0x7FA5;
        cpu.y = 0x05;
        REQUIRE(cpu.IZY(bus) == 0);
        REQUIRE(cpu.addr_abs == 0xB00B + 0x05);
        REQUIRE(cpu.pc == 0x7FA6);

        bus.write(0x7FA5, 0xFF);
        bus.write(0x00FF, 0x0B);
        bus.write(0x0000, 0xB0);
        bus.write(0x0100, 0xC0);

        cpu.pc = 0x7FA5;
        REQUIRE(cpu.IZY(bus) == 0);
        REQUIRE(cpu.addr_abs == 0xB00B + 0x05);
        REQUIRE(cpu.pc == 0x7FA6);
    }
}
