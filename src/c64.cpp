#include "c64/c64.hpp"
#include "c64/basic.hpp"
#include "c64/kernal.hpp"
#include "c64/characters.hpp"

C64::C64() : ram{} {
    std::copy(std::begin(basic_bin), std::end(basic_bin), std::begin(basic_rom));
    std::copy(std::begin(chars_bin), std::end(chars_bin), std::begin(char_rom));
    std::copy(std::begin(kernal_bin), std::end(kernal_bin), std::begin(kernal_rom));
}

void C64::write(uint16_t addr, uint8_t value) {
    if (addr == 0x0000) {
        printf("[CPU IO $0] %02X\n", value);
    } else if (addr == 0x0001) {
        printf("[CPU IO $1] %02X\n", value);
    } else if (0xD000 <= addr && addr <= 0xDFFF) {
        if (0xD000 <= addr && addr <= 0xD3FF) {
            printf("[VIC-II] ");
        } else if (0xD400 <= addr && addr <= 0xD7FF) {
            printf("[SID] ");
        } else if (0xD800 <= addr && addr <= 0xDBFF) {
            printf("[COLOR RAM] ");
        } else if (0xDC00 <= addr && addr <= 0xDCFF) {
            printf("[CIA 1] ");
        } else if (0xDD00 <= addr && addr <= 0xDDFF) {
            printf("[CIA 2] ");
        } else if (0xDE00 <= addr && addr <= 0xDEFF) {
            printf("[I/O 1] ");
        } else if (0xDF00 <= addr && addr <= 0xDFFF) {
            printf("[I/O 2] ");
        }
        printf("Writing I/O: $%04X <- %02X\n", addr, value);
    } else if (0x0400 <= addr && addr <= 0x07E7) {
//        printf("[Screen] $%04X <- %02X\n", addr, value);
    }
    ram[addr] = value;
}

uint8_t C64::read(uint16_t addr, bool read_only) {
    uint8_t port_reg = ram[0x0001];
    bool charen = (port_reg & 0b100u) == 0b100u;
    bool hiram = (port_reg & 0b010u) == 0b010u;
    bool loram = (port_reg & 0b001u) == 0b001u;

    if ((hiram && loram) && (0xA000 <= addr && addr <= 0xBFFF)) {
        return basic_rom[addr - 0xA000];
    } else if (charen && (hiram || loram) && (0xD000 <= addr && addr <= 0xDFFF)) {
        if (0xD000 <= addr && addr <= 0xD3FF) {
            printf("[VIC-II] ");
            if (addr == 0xD012) { // raster counter
                return 0x00;
            }
        } else if (0xD400 <= addr && addr <= 0xD7FF) {
            printf("[SID] ");
        } else if (0xD800 <= addr && addr <= 0xDBFF) {
            printf("[COLOR RAM] ");
        } else if (0xDC00 <= addr && addr <= 0xDCFF) {
            printf("[CIA 1] ");
        } else if (0xDD00 <= addr && addr <= 0xDDFF) {
            printf("[CIA 2] ");
        } else if (0xDE00 <= addr && addr <= 0xDEFF) {
            printf("[I/O 1] ");
        } else if (0xDF00 <= addr && addr <= 0xDFFF) {
            printf("[I/O 2] ");
        }
        printf("Reading I/O: $%04X\n", addr);
    } else if ((hiram || loram) && (0xD000 <= addr && addr <= 0xDFFF)) {
        return char_rom[addr - 0xD000];
    } else if (hiram && (0xE000 <= addr && addr <= 0xFFFF)) {
        return kernal_rom[addr - 0xE000];
    }
    return ram[addr];
}

void C64::reset() {
    ram[0x0001] = 0b010;
    cpu.reset(*this);
}

bool C64::clock() {
    cpu.clock(*this);

    if (interrupt_state == Interrupt::NMI) {
        cpu.nmi(*this);
    } else if (interrupt_state == Interrupt::IRQ) {
        cpu.irq(*this);
    }

    interrupt_state = Interrupt::None;
    system_clock++;
    return cpu.complete();
}

void C64::interrupt(Interrupt interrupt) {
    interrupt_state = interrupt;
}

CPU6502 const& C64::get_cpu() const {
    return cpu;
}
