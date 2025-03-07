#include "bus.h"
#include <cstddef>

/*
 * Register - a union that represents a 16-bit register, as well as its lower and upper 8-bit registers.
 **/
union Register {
    uint16_t r16;
    struct {
        uint8_t low;
        uint8_t high;
    } r8;
};

struct CPUFlags { int z, n, h, c; };

/*
 * CPU - emulates the SM83 instruction set
 * Reference: https://gbdev.io/pandocs/CPU_Registers_and_Flags.html
 **/
class CPU {
public:
    CPU(Bus *bus) : m_bus(bus) { }

    void step();

private:
    Bus *m_bus = nullptr;
    Register AF;               // Accumulator & flags
    Register BC, DE, HL;       // General purpose registers
    Register PC = { 0x0100 };  // Program counter (start at 0x0100, according to pandocs)
    Register SP = { 0xFFFE };  // Stack pointer (start at 0xFFFE, according to pandocs)

    Register *get_r16_from_opcode(uint8_t, size_t);
    uint8_t  *get_r8_from_opcode(uint8_t, size_t);

    template<typename T> void add(T, T, T &, bool);
    template<typename T> void sub(T, T, T &, bool);

    void set_flags(CPUFlags);
};

