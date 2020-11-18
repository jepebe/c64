#include <c64/instrumentation.hpp>
#include <fmt/core.h>
#include "c64/pyc64.hpp"

#include <pybind11/pybind11.h>
namespace py = pybind11;

pyC64::pyC64() : c64() {

}

const CPU6502& pyC64::cpu() const {
    return c64.get_cpu();
}

std::string pyC64::disassemble(uint16_t addr) {
    auto cpu = c64.get_cpu();
    auto opcode = c64.read(addr, true);
    auto info = cpu.instruction_info(opcode);
    auto addr_str = c64tools::address(cpu, c64);
    auto instruction = info.instruction;
    auto ns = info.non_standard ? "*" : " ";
    auto addr_mode = info.addr_mode;

    return fmt::format("${:04X} {}{} {:<7} [{}]",
                       addr, ns, instruction, addr_str, addr_mode);
}

bool pyC64::clock() {
    return c64.clock();;
}

void pyC64::reset() {
    c64.reset();
}

PYBIND11_MODULE(pyC64, m) {
    auto cpu = py::class_<CPU6502>(m, "_cpu");
    cpu.def_readonly("a", &CPU6502::a);
    cpu.def_readonly("x", &CPU6502::x);
    cpu.def_readonly("y", &CPU6502::y);
    cpu.def_readonly("pc", &CPU6502::pc);
    cpu.def_readonly("stkp", &CPU6502::stkp);
    cpu.def_readonly("status", &CPU6502::status);

    auto pyc64 = py::class_<pyC64>(m, "pyC64");
    pyc64.def("clock", &pyC64::clock);
    pyc64.def("cpu", &pyC64::cpu);
    pyc64.def("disassemble", &pyC64::disassemble);
    pyc64.def("reset", &pyC64::reset);

    m.doc() = "C64 Emulator Module";
}