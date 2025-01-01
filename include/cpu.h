/* Reference: https://gbdev.io/pandocs/CPU_Registers_and_Flags.html */
#include "bus.h"

struct CPUFlags { int z, n, h, c; };

class CPU {
public:
    CPU(Bus *bus) : m_bus(bus) { }

    void cycle();
    void set_flags(CPUFlags);

    template<typename T> void add(T, T, T &, bool);
    template<typename T> void sub(T, T, T &, bool);

private:
    uint16_t AF;               // Accumulator & flags

    uint16_t BC, DE, HL;       // General purpose

    uint16_t SP = 0xFFFE;      // Stack pointer
    uint16_t PC = 0x0100;      // Program counter
    
    Bus *m_bus;                // Address resolver
};
