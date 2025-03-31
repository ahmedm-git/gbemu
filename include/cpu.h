#include <stdint.h>
#include "bus.h"

union Register {
    uint16_t r16;
    struct {
        uint8_t lo;
        uint8_t hi;
    } r8;
};

union CPUFlags {
    uint8_t flags;
    struct {
        uint8_t c : 1; // Carry
        uint8_t h : 1; // Half Carry
        uint8_t n : 1; // Subtract
        uint8_t z : 1; // Zero
        uint8_t unused : 4;
    } bits;
};

struct CPUState {
    Register AF {0};
    Register BC {0};
    Register DE {0};
    Register HL {0};
    Register SP {0};
    Register PC {0};
    CPUFlags FLAGS {0};

    uint8_t  IME = 0;
    uint8_t  IF = 0;
    uint8_t  IE = 0;

    uint64_t MCYCLES = 0;
};

/*
 * CPU - GB SM83 CPU class
 *
 * References: 
 * - https://gbdev.io/gb-opcodes/optables/octal
 * - https://gbdev.io/pandocs/Specifications.html
 */
class CPU {
public:
    friend class TestCPU;  // For testing of private methods.

    CPU() { m_bus = std::make_unique<Bus>(); }
    CPU(Bus *bus) : m_bus(bus) {}

    void step();
    void reset();
    void set_state(const CPUState &state) { m_state = state; }

    CPUState &get_state() { return m_state; }
    uint8_t fetch();

private:
    CPUState m_state;
    std::unique_ptr<Bus> m_bus;
    uint8_t m_cycles_to_wait = 0;

    /**
     * get_register_ref_r8 - Gets the 8-bit register pointer from an opcode that includes an 8-bit register.
     *                       The default start/end positions assume the last three bits of the opcode are the register.
     * @opcode:    the opcode to extract the register from.
     * @start_pos: the starting position of the opcode.
     *
     * Return: the pointer to the register.
     **/
    uint8_t *get_r8_from_opcode(uint8_t opcode, size_t start_pos = 0)
    {
        uint8_t *r8;
        switch ((opcode >> start_pos) & 0b111) {
            case 0b000:
                r8 = &m_state.BC.r8.hi;
                break;
            case 0b001:
                r8 = &m_state.BC.r8.lo;
                break;
            case 0b010:
                r8 = &m_state.DE.r8.hi;
                break;
            case 0b011:
                r8 = &m_state.DE.r8.lo;
                break;
            case 0b100: 
                r8 = &m_state.HL.r8.hi;
                break;
            case 0b101:
                r8 = &m_state.HL.r8.lo;
                break;
            case 0b111: 
                r8 = &m_state.AF.r8.hi;
                break;
            default:  // 0b110 is [HL], not a register
                throw std::invalid_argument("Invalid register selection");
        }
        return r8;
    }

    /**
     * get_register_ref_r16 - Gets the full register pointer from an opcode that includes a 16-bit register.
     *                        The default start/end positions assume the last two bits of the opcode are the register.
     * @opcode:    the opcode to extract the register from.
     * @start_pos: the starting position of the opcode.
     *
     * Return: the pointer to the register.
     **/
    Register *get_r16_from_opcode(uint8_t opcode, size_t start_pos = 0)
    {
        Register *reg;
        switch ((opcode >> start_pos) & 0b11) {
            case 0b00: reg = &m_state.BC; break;
            case 0b01: reg = &m_state.DE; break;
            case 0b10: reg = &m_state.HL; break;
            case 0b11: reg = &m_state.SP; break;
            default: return nullptr;
        }
        return reg;
    }

    /**
     * add - Adds two values together and sets the flags accordingly.
     * @a:          the first value to add.
     * @b:          the second value to add.
     * @result:     the result of the addition.
     **/
    template<typename T>
    void add(T a, T b, T &result) {
        bool mode_16bit = sizeof(T) == 2;
        uint16_t bitmask = mode_16bit ? 0xFFFF : 0xFF;
        uint16_t c = a + b;
        result = c & bitmask;
        m_state.FLAGS = CPUFlags {
            .bits = {
                .c = (c > bitmask) ? 1 : 0,
                .h = (((a & 0xF) + (b & 0xF)) > 0xF) ? 1 : 0,
                .n = 0,
                .z = (result == 0) ? 1 : 0
            }
        };
    }

    /**
     * sub - Subtracts two values together and sets the flags accordingly.
     * @a:          the first value to subtract.
     * @b:          the second value to subtract.
     * @result:     the result of the subtraction.
     **/
    template<typename T>
    void sub(T a, T b, T &result) {
        bool mode_16bit = sizeof(T) == 2;
        uint16_t bitmask = mode_16bit ? 0xFFFF : 0xFF;
        int16_t c = a - b;
        result = c & bitmask;
        m_state.FLAGS = CPUFlags {
            .bits = {
                .c = (c < 0) ? 1 : 0,
                .h = ((b & 0xF) > (a & 0xF)) ? 1 : 0,
                .n = 1,
                .z = (result == 0) ? 1 : 0
            }
        };
    }
};
