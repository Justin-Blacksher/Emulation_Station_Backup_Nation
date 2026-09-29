#pragma once
#include <cstdint>
#include "bus.h"

class CPU {
    public:
    CPU();
    ~CPU() = default;

    // Core registers
    u8t a = 0x00;           // Accumulator
    u8t x = 0x00;           // X Register
    u8t y = 0x00;           // Y Register
    u8t stkp = 0xFD;        // Stack Pointer
    u16t pc = 0x0000;       // Program counter
    u8t status = 0x00;      // Status Register

    // Status Flag Masks
    enum FLAGS {
        C = (1 << 0),           // Carry Flag
        Z = (1 << 1),           // Zero Flag
        I = (1 << 2),           // Interrupt Disable
        D = (1 << 3),           // Decimal Mode
        B = (1 << 4),           // Break Command
        U = (1 << 5),           // Unused (Always 1)
        V = (1 << 6),           // Overflow Flag
        N = (1 << 7),           // Negative Flag
    };           

    void reset(Bus& bus);
    void clock(Bus& bus);

    private:
        // Auxiliary variables to track current state
        u8t fetched = 0x00;     // Represents the working input value for an instruction
        u16t temp = 0x0000;     // A general purpose helper variable
        u16t addr_abs = 0x0000; // Absolute memory address target
        u16t addr_rel = 0x0000; // Relative address for branching
        u8t opcode = 0x00;      // Current instruction opcode
        u8t cycles = 0;         // Remaining cycles for current instruction

        // --- Addressing Modes ---
        // These return 1 if an additional cycle is required
        u8t IMP();              // Implied
        u8t IMM();              // Immediate
        u8t ZP0();              // Zero Page
        u8t ZPX();              // Zero page with x offset
        u8t ZPY();              // Zero page with y offset
        u8t ABS();              // Absolute
        u8t ABX();              // Absolute with X offset
        u8t ABY();              // Absolute with Y offset
        u8t REL();              // Relative (Branches)
        u8t IND();              // Indirect (JMP)
        u8t IZX();              // Indirect X
        u8t IZY();              // Indirect Y

        // Instructions
        u8t LDA();              // Load Accumulator
        u8t STA();              // Store Accumulator
        u8t NOP();              // No Operation
        u8t BRK();              // Force Interrupt

        // Helper method to set/clear flags
        u8t get_flag(FLAGS f);
        void set_flag(FLAGS f, bool v);

        // Instruction lookup table
        struct Instruction {
            const char* name;
            u8t (CPU::*addressof_op)(void) = nullptr;
            u8t (CPU::*addressof_ins)(void) = nullptr;
            u8t cycles = 0;
        };

        // We'll populate a lookup table later
};