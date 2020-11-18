#ifndef NES_CPU_6502_HPP
#define NES_CPU_6502_HPP

#include <cstdint>
#include <string>

enum Interrupt {
    None,
    IRQ,
    NMI
};

class CPUIO {
public:
    virtual void write(uint16_t addr, uint8_t value) = 0;

    virtual uint8_t read(uint16_t addr, bool read_only) = 0;

    virtual void interrupt(Interrupt interrupt) = 0;

    uint8_t read(uint16_t addr) { return read(addr, false); }
};

enum Flags6502 : uint8_t {
    C = 0x01, // Carry Bit
    Z = 0x02, // Zero
    I = 0x04, // Disable Interrupts
    D = 0x08, // Decimal Mode (unused in this implementation)
    B = 0x10, // Break
    U = 0x20, // Unused
    V = 0x40, // Overflow
    N = 0x80, // Negative
};

bool is_flag_set(uint8_t flag, uint8_t flags);

struct InstructionInfo {
    const char * instruction;
    const char * addr_mode;
    bool non_standard;
};

class CPU6502 {
public:
    uint8_t a = 0x00;
    uint8_t x = 0x00;
    uint8_t y = 0x00;
    uint8_t stkp = 0x00;
    uint16_t pc = 0x0000;
    uint8_t status = 0x00;

    uint16_t addr_abs = 0x0000;
    uint8_t addr_rel = 0x00;
    uint8_t cycles = 0x00;

    uint8_t opcode = 0x00;
    uint64_t clock_count = 0;
    uint8_t implied = 0x00;
    bool implied_has_value = false;

    CPU6502();

    virtual ~CPU6502();

    void clock(CPUIO &bus);

    void reset(CPUIO &bus);

    void irq(CPUIO &bus);

    void nmi(CPUIO &bus);

    [[nodiscard]] bool complete() const;

    [[nodiscard]] uint8_t get_flag(Flags6502 flag) const;

    [[nodiscard]] bool is_status_flag_set(Flags6502 flag) const;

    [[nodiscard]] virtual InstructionInfo instruction_info(uint8_t opcode) const;

protected:
    virtual uint8_t run_instruction(CPUIO &bus);

    uint8_t fetch(CPUIO &bus);

    void set_status_flag(Flags6502 flag, bool value);

    void push_value_on_stack(CPUIO &bus, uint8_t value);

    uint8_t pop_value_from_stack(CPUIO &bus);

    void push_interrupt_state_on_stack(CPUIO & bus);

    void push_program_counter_on_stack(CPUIO &bus);

    void pop_program_counter_from_stack(CPUIO &bus);

    void load_program_counter_from_addr(CPUIO &bus, uint16_t addr);

    bool is_negative(uint8_t);

    void add(uint8_t m);

    void add_bdc(uint8_t m, int8_t sign);

    void branch();


    // *** --- Addressing Modes --- ***
    /// Implied -> the fetched value is the _a_ register
    uint8_t IMP(CPUIO &bus);

    /// Immediate -> the value pointed to by the program counter
    uint8_t IMM(CPUIO &bus);

    /// Zero Page 0 -> The address is in the range $0000-$00FF
    uint8_t ZP0(CPUIO &bus);

    /// Zero Page + X -> An address is read from memory and _x_ is added.
    /// The final address is in the range $0000-$00FF (wrap around).
    uint8_t ZPX(CPUIO &bus);

    /// Zero Page + Y -> An address is read from memory and _y_ is added.
    /// The final address is in the range $0000-$00FF (wrap around).
    uint8_t ZPY(CPUIO &bus);

    /// Relative -> A value is read from memory and stored for branching jump operations.
    uint8_t REL(CPUIO &bus);

    /// Absolute -> Two values are read from memory and used as an address.
    uint8_t ABS(CPUIO &bus);

    /// Absolute + x -> Two values are read from memory then _x_ is added
    /// and used as an address.
    uint8_t ABX(CPUIO &bus);

    /// Absolute + y -> Two values are read from memory then _y_ is added
    /// and used as an address.
    uint8_t ABY(CPUIO &bus);

    /// Indirect -> Two values are read from memory and used as an address to
    /// read the actual address.
    virtual uint8_t IND(CPUIO &bus);

    /// Indirect Zero Page + X -> A value is read from memory and _x_ is added, then
    /// the actual address is read from the zero page.
    uint8_t IZX(CPUIO &bus);

    /// Indirect Zero Page + Y -> A value is read from memory and the actual
    /// address is read from the zero page then _y_ is added.
    uint8_t IZY(CPUIO &bus);


    // *** --- Instructions --- ***

    /// Add with carry
    uint8_t ADC(CPUIO &bus);

    /// ANDs the contents of the A register with an immediate
    /// value and then moves bit 7 of A into the Carry flag
    uint8_t ANC(CPUIO &bus);

    /// AND (with accumulator)
    uint8_t AND(CPUIO &bus);

    /// arithmetic shift left
    uint8_t ASL(CPUIO &bus);

    /// branch on carry clear
    uint8_t BCC(CPUIO &bus);

    /// branch on carry set
    uint8_t BCS(CPUIO &bus);

    /// branch on equal (zero set)
    uint8_t BEQ(CPUIO &bus);

    /// bit test
    uint8_t BIT(CPUIO &bus);

    /// branch on minus (negative set)
    uint8_t BMI(CPUIO &bus);

    /// branch on not equal (zero clear)
    uint8_t BNE(CPUIO &bus);

    /// branch on plus (negative clear)
    uint8_t BPL(CPUIO &bus);

    /// break / interrupt
    virtual uint8_t BRK(CPUIO &bus);

    /// branch on overflow clear
    uint8_t BVC(CPUIO &bus);

    /// branch on overflow set
    uint8_t BVS(CPUIO &bus);

    /// clear carry
    uint8_t CLC(CPUIO &bus);

    /// clear decimal
    uint8_t CLD(CPUIO &bus);

    /// clear interrupt disable
    uint8_t CLI(CPUIO &bus);

    /// clear overflow
    uint8_t CLV(CPUIO &bus);

    /// compare (with accumulator)
    uint8_t CMP(CPUIO &bus);

    /// compare with X
    uint8_t CPX(CPUIO &bus);

    /// compare with Y
    uint8_t CPY(CPUIO &bus);

    /// decrement and compare with Accumulator
    uint8_t DCP(CPUIO &bus);

    /// decrement
    uint8_t DEC(CPUIO &bus);

    /// decrement X
    uint8_t DEX(CPUIO &bus);

    /// decrement Y
    uint8_t DEY(CPUIO &bus);

    /// exclusive or (with accumulator)
    uint8_t EOR(CPUIO &bus);

    /// increment
    uint8_t INC(CPUIO &bus);

    /// increment X
    uint8_t INX(CPUIO &bus);

    /// increment Y
    uint8_t INY(CPUIO &bus);

    /// Increment memory with one and then subtract memory from Accumulator
    uint8_t ISB(CPUIO &bus);

    /// jump
    uint8_t JMP(CPUIO &bus);

    /// jump subroutine
    uint8_t JSR(CPUIO &bus);

    /// * load accumulator and x with memory contents
    uint8_t LAX(CPUIO &bus);

    /// load accumulator
    uint8_t LDA(CPUIO &bus);

    /// Load X
    uint8_t LDX(CPUIO &bus);

    /// Load Y
    uint8_t LDY(CPUIO &bus);

    /// logical shift right
    uint8_t LSR(CPUIO &bus);

    /// no operation
    uint8_t NOP(CPUIO &bus);

    /// or with accumulator
    uint8_t ORA(CPUIO &bus);

    /// push accumulator
    uint8_t PHA(CPUIO &bus);

    /// push processor status (SR)
    uint8_t PHP(CPUIO &bus);

    /// pull accumulator
    uint8_t PLA(CPUIO &bus);

    /// pull processor status (SR)
    uint8_t PLP(CPUIO &bus);

    /// rotate memory left and then AND accumulator with memory
    uint8_t RLA(CPUIO &bus);

    /// rotate left
    uint8_t ROL(CPUIO &bus);

    /// rotate right
    uint8_t ROR(CPUIO &bus);

    /// Rotate memory one bit to the right and then add memory to accumulator
    uint8_t RRA(CPUIO &bus);

    /// return from interrupt
    uint8_t RTI(CPUIO &bus);

    /// return from subroutine
    uint8_t RTS(CPUIO &bus);

    /// AND X with Accumulator and store in memory
    uint8_t SAX(CPUIO &bus);

    /// Subtract with borrow
    uint8_t SBC(CPUIO &bus);

    /// Set Carry
    uint8_t SEC(CPUIO &bus);

    /// Set Decimal
    uint8_t SED(CPUIO &bus);

    /// set interrupt disable
    uint8_t SEI(CPUIO &bus);

    /// Shift memory left and then OR accumulator with memory
    uint8_t SLO(CPUIO &bus);

    /// Shift memory one bit right then XOR with accumulator
    uint8_t SRE(CPUIO &bus);

    /// store accumulator
    uint8_t STA(CPUIO &bus);

    /// store X
    uint8_t STX(CPUIO &bus);

    /// store Y
    uint8_t STY(CPUIO &bus);

    /// transfer accumulator to X
    uint8_t TAX(CPUIO &bus);

    /// transfer accumulator to Y
    uint8_t TAY(CPUIO &bus);

    /// transfer stack pointer to X
    uint8_t TSX(CPUIO &bus);

    /// transfer X to accumulator
    uint8_t TXA(CPUIO &bus);

    /// transfer X to stack pointer
    uint8_t TXS(CPUIO &bus);

    /// transfer Y to accumulator
    uint8_t TYA(CPUIO &bus);

    /// Undefined opcode
    uint8_t XXX(CPUIO &bus);

};


#endif
