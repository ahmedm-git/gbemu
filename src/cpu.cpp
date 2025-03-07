#include <stdexcept>
#include "cpu.h"

/*
 * get_register_ref_r8 - Gets the r8 pointer from an opcode that includes an 8-bit register.
 *                       Note that this function returns a pointer to a uint8_t, not a Register.
 * @opcode:    the opcode to extract the register from.
 * @start_pos: the starting position of the opcode, beginning from the least significant bit.
 *
 * Example: opcode is 0b00101110 (in the form LD r8, n8), start_pos is 3.
 * Shift the opcode to the right by start_pos: 0b00101110 >> 3 = 0b00000101
 * Mask the opcode with 0b111: 0b101 & 0b111 = 0b101
 * The result is 0b101, which is the index of the register.
 *
 * Return: the pointer to the r8 register.
 */
uint8_t *CPU::get_r8_from_opcode(uint8_t opcode, size_t start_pos = 0)
{
    uint8_t *r8;
    switch ((opcode >> start_pos) & 0b111) {
        case 0b000:
            r8 = &BC.r8.high;
            break;
        case 0b001:
            r8 = &BC.r8.low;
            break;
        case 0b010:
            r8 = &DE.r8.high;
            break;
        case 0b011:
            r8 = &DE.r8.low;
            break;
        case 0b100: 
            r8 = &HL.r8.high;
            break;
        case 0b101:
            r8 = &HL.r8.low;
            break;
        case 0b111: 
            r8 = &AF.r8.high;
            break;
        default:  // 0b110 is [HL], not a register
            return nullptr;
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
Register *CPU::get_r16_from_opcode(uint8_t opcode, size_t start_pos = 0)
{
    Register *reg;
    switch ((opcode >> start_pos) & 0b11) {
        case 0b00: reg = &BC; break;
        case 0b01: reg = &DE; break;
        case 0b10: reg = &HL; break;
        case 0b11: reg = &SP; break;
        default: return nullptr;
    }
    return reg;
}

/**
 * add - Adds two values together and sets the flags accordingly.
 * @a:          the first value to add.
 * @b:          the second value to add.
 * @result:     the result of the addition.
 * @mode_16bit: whether the addition is 16-bit or not.
 **/
template<typename T>
void CPU::add(T a, T b, T &result, bool mode_16bit)
{
    uint16_t bitmask = mode_16bit ? 0xFFFF : 0xFF;
    uint16_t c = a + b;
    result = c & bitmask;
    set_flags({
        .z = (result == 0) ? 1 : 0,
        .n = 0,
        .h = (((a & 0xF) + (b & 0xF)) > 0xF) ? 1 : 0,
        .c = (c > bitmask) ? 1 : 0
    });
}

/**
 * sub - Subtracts two values together and sets the flags accordingly.
 * @a:          the first value to subtract.
 * @b:          the second value to subtract.
 * @result:     the result of the subtraction.
 * @mode_16bit: whether the subtraction is 16-bit or not.
 **/
template<typename T>
void CPU::sub(T a, T b, T &result, bool mode_16bit)
{
    uint16_t bitmask = mode_16bit ? 0xFFFF : 0xFF;
    int16_t c = a - b;
    result = c & bitmask;
    set_flags({
        .z = (result == 0) ? 1 : 0,
        .n = 1,
        .h = ((b & 0xF) > (a & 0xF)) ? 1 : 0,
        .c = (c < 0) ? 1 : 0
    });
}

/**
 * set_flags - Given CPUFlags, set AF.lower to the flags.
 * @flags: the flags to set.
 **/
void CPU::set_flags(CPUFlags flags)
{
    uint8_t new_flags = AF.r8.low;
    if (flags.z >= 0) new_flags = (new_flags & ~0x80) | (flags.z << 7);
    if (flags.n >= 0) new_flags = (new_flags & ~0x40) | (flags.n << 6);
    if (flags.h >= 0) new_flags = (new_flags & ~0x20) | (flags.h << 5);
    if (flags.c >= 0) new_flags = (new_flags & ~0x10) | (flags.c << 4);
    AF = Register { .r8 = { AF.r8.high, new_flags } };
}

/**
 * step - Steps through memory and executes the opcode.
 * Reference: https://gbdev.io/gb-opcodes/optables/octal/
 **/
void CPU::step()
{
    uint8_t opcode = m_bus->read_n8(PC.r16++);

    switch (opcode) {
        case 0:
            break;

        case 0166:
            exit(0);
            break;

        /**************************************** 8-bit loads ****************************************
         * 0100-0177: {
         *              LD r8, r8
         *              LD r8, [HL]
         *              LD [HL], r
         *            }
         * 0002-0062: LD [r16], A
         * 0012-0072: LD A, [r16]
         * 0006-0076: LD r8, n8
         * 0340:      LDH [a8], A
         * 0360:      LDH A, [a8]
         * 0342:      LDH [C], A
         * 0352:      LD [a16], A
         * 0362:      LDH A, [C]
         * 0372:      LD A, [a16]
         ********************************************************************************************/

        case 0100: case 0101: case 0102: case 0103: case 0104: case 0105: case 0106: case 0107:
        case 0110: case 0111: case 0112: case 0113: case 0114: case 0115: case 0116: case 0117:
        case 0120: case 0121: case 0122: case 0123: case 0124: case 0125: case 0126: case 0127:
        case 0130: case 0131: case 0132: case 0133: case 0134: case 0135: case 0136: case 0137:
        case 0140: case 0141: case 0142: case 0143: case 0144: case 0145: case 0146: case 0147:
        case 0150: case 0151: case 0152: case 0153: case 0154: case 0155: case 0156: case 0157:
        case 0160: case 0161: case 0162: case 0163: case 0164: case 0165: case 0167:
        case 0170: case 0171: case 0172: case 0173: case 0174: case 0175: case 0176: case 0177:
            if ((opcode % 010) == 06) {
                /* LD [HL], r8 */
                m_bus->write_n8(HL.r16, *get_r8_from_opcode(opcode));
            } else if ((opcode >> 4) == 016) {
                /* LD r8, [HL] */
                *get_r8_from_opcode(opcode) = m_bus->read_n8(HL.r16);
            } else {
                /* LD r8, r8 */
                *get_r8_from_opcode(opcode) = *get_r8_from_opcode(opcode, 3);
            }
            break;

        /* LD [r16], A */
        case 0002: case 0024: case 0042: case 0062:
            m_bus->write_n8(get_r16_from_opcode(opcode, 3)->r16, AF.r8.high);
            if (opcode == 062) {
                HL.r16--;
            } else if (opcode == 042) {
                HL.r16++;
            }
            break;

        /* LD A, [r16] */
        case 0012: case 0032: case 0052: case 0072:
            AF.r8.high = m_bus->read_n8(get_r16_from_opcode(opcode)->r16);
            if (opcode == 072) {
                HL.r16--;
            } else if (opcode == 052) {
                HL.r16++;
            }
            break;

        /* LD r8, n8 */
        case 0006: case 0016: case 0026: case 0036: case 0046: case 0056: case 0066: case 0076:
            *get_r8_from_opcode(opcode, 0) = m_bus->read_n8(PC.r16++);
            break;

        /* LDH [a8], A */
        case 0340:
            m_bus->write_n8(0xFF00 + m_bus->read_n8(PC.r16++), AF.r8.high);
            break;

        /* LDH A, [a8] */
        case 0360:
            AF.r8.high = m_bus->read_n8(0xFF00 + m_bus->read_n8(PC.r16++));
            break;

        /* LDH [C], A */
        case 0342:
            m_bus->write_n8(0xFF00 + BC.r8.low, AF.r8.high);
            break;

        /* LD [a16], A */
        case 0352:
            m_bus->write_n8(m_bus->read_n16(PC.r16 += 2), AF.r8.high);
            break;

        /* LDH A, [C] */
        case 0362:
            AF.r8.high = m_bus->read_n8(0xFF00 + BC.r8.low);
            break;

        /* LD A, [a16] */
        case 0372:
            AF.r8.high = m_bus->read_n8(m_bus->read_n16(PC.r16 += 2));
            break;

        /**************************************** 16-bit loads ****************************************
         * 0001, 0021, 0041, 0061: LD r16, n16
         * 0010: LD [a16], SP
         * 0370: LD HL, SP+n8
         * 0301, 0321, 0341, 0361: POP r16
         * 0371: LD SP, HL
         *********************************************************************************************/

        /* LD BC, n16 */
        case 0001:
            BC.r16 = m_bus->read_n16(PC.r16 += 2);
            break;

        /* LD DE, n16 */
        case 0021:
            DE.r16 = m_bus->read_n16(PC.r16 += 2);
            break;

        /* LD HL, n16 */
        case 0041:
            HL.r16 = m_bus->read_n16(PC.r16 += 2);
            break;

        /* LD SP, n16 */
        case 0061:
            SP.r16 = m_bus->read_n16(PC.r16 += 2);
            break;

        /* LD [a16], SP */
        case 0010:
            m_bus->write_n16(m_bus->read_n16(PC.r16 += 2), SP.r16);
            break;

        /* LD HL, SP+n8 */
        case 0370:
            HL.r16 = SP.r16 + m_bus->read_n8(PC.r16++);
            break;

        /* POP BC */
        case 0301:
            BC.r16 = m_bus->read_n16(SP.r16 += 2);
            break;

        /* POP DE */
        case 0321:
            DE.r16 = m_bus->read_n16(SP.r16 += 2);
            break;

        /* POP HL */
        case 0341:
            HL.r16 = m_bus->read_n16(SP.r16 += 2);
            break;

        /* POP AF */
        case 0361:
            AF.r16 = m_bus->read_n16(SP.r16 += 2);
            break;

        /* LD SP, HL */
        case 0371:
            SP.r16 = HL.r16;
            break;

        /* PUSH BC */
        case 0305:
            m_bus->write_n16(SP.r16 -= 2, BC.r16);
            break;

        /* PUSH DE */
        case 0325:
            m_bus->write_n16(SP.r16 -= 2, DE.r16);
            break;

        /* PUSH HL */
        case 0345:
            m_bus->write_n16(SP.r16 -= 2, HL.r16);
            break;

        /* PUSH AF */
        case 0365:
            m_bus->write_n16(SP.r16 -= 2, AF.r16);
            break;

        /* 8-bit ALU */

        /* 16-bit ALU */

        /* 8-bit shift, rotate, and bit operations */

        /* jumps/calls */

        /* misc/control */

        default:
            throw std::runtime_error("Received unknown opcode");
    }
}

