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
 **/
template<typename T>
void CPU::add(T a, T b, T &result)
{
    bool mode_16bit = sizeof(T) == 2;
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
 **/
template<typename T>
void CPU::sub(T a, T b, T &result)
{
    bool mode_16bit = sizeof(T) == 2;
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
    m_flags = flags;
}

/**
 * step - Steps through memory and executes the opcode.
 * Reference: https://gbdev.io/gb-opcodes/optables/octal/
 **/
void CPU::step()
{
    uint8_t opcode = m_bus->read_n8(PC.r16++);

    switch (opcode) {
    /********************************************************************************************
     * 8-bit loads
     ********************************************************************************************/

    case 0100 ... 0165: case 0167 ... 0177:
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
    case 0002: case 0022: case 0042: case 0062:
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

    /********************************************************************************************
     * 16-bit loads
     ********************************************************************************************/

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

    /********************************************************************************************
     * 8-bit arithmetic
     ********************************************************************************************/

    /* INC r8 */
    case 0004: case 0014: case 0024: case 0034: case 0044: case 0054: case 0064: case 0074:
    {
        uint8_t intermediate, target = *get_r8_from_opcode(opcode, 3);
        if (opcode == 0064) {  // INC [HL]
            target = m_bus->read_n8(HL.r16);
        }
        add<uint8_t>(
            target,
            1,
            intermediate
        );
        if (opcode == 0064) {  // INC [HL]
            m_bus->write_n8(HL.r16, intermediate);
        } else {
            *get_r8_from_opcode(opcode, 3) = intermediate;
        }
        break;
    }

    /* DEC r8 */
    case 0005: case 0015: case 0025: case 0035: case 0045: case 0055: case 0065: case 0075:
    {
        uint8_t intermediate, target = *get_r8_from_opcode(opcode, 3);
        if (opcode == 0065) {  // DEC [HL]
            target = m_bus->read_n8(HL.r16);
        }
        sub<uint8_t>(
            target,
            1,
            intermediate
        );
        if (opcode == 0065) {  // DEC [HL]
            m_bus->write_n8(HL.r16, intermediate);
        } else {
            *get_r8_from_opcode(opcode, 3) = intermediate;
        }
        break;
    }

    /* DAA */
    case 0047:
    {
        uint8_t adjustment = 0;
        if (m_flags.n) {
            if (m_flags.h) {
                adjustment += 0x6;
            }
            if (m_flags.c) {
                adjustment += 0x60;
            }
            AF.r8.high -= adjustment;
        } else {
            if (m_flags.h || (AF.r8.high & 0xF) > 0x9) {
                adjustment += 0x6;
            }
            if (m_flags.c || AF.r8.high > 0x99) {
                adjustment += 0x60;
                m_flags.c = 1;
            }
            AF.r8.high += adjustment;
        }
        break;
    }

    /* CPL (bitwise not for A) */
    case 0057:
        AF.r8.high = ~AF.r8.high;
        break;

    /* SCF (set carry flag) */
    case 0067:
        set_flags({ .z = m_flags.z, .n = 0, .h = 0, .c = 1 });
        break;

    /* CCF (complement carry flag) */
    case 0077:
        set_flags({ .z = m_flags.z, .n = 0, .h = 0, .c = !m_flags.c });
        break;

    /* ADD A, r8 */
    case 0200 ... 0207:
    {
        uint8_t intermediate, target = *get_r8_from_opcode(opcode);
        if (opcode == 0206) {  // ADD A, [HL]
            target = m_bus->read_n8(HL.r16);
        }
        add<uint8_t>(
            AF.r8.high,
            target,
            intermediate
        );
        AF.r8.high = intermediate;
        break;
    }

    /* ADC A, r8 */
    case 0210 ... 0217:
    {
        uint8_t intermediate, target = *get_r8_from_opcode(opcode);
        if (opcode == 0216) {  // ADC A, [HL]
            target = m_bus->read_n8(HL.r16);
        }
        add<uint8_t>(
            AF.r8.high,
            target + m_flags.c,
            intermediate
        );
        AF.r8.high = intermediate;
        break;
    }

    /* SUB A, r8 */
    case 0220 ... 0227:
    {
        uint8_t intermediate, target = *get_r8_from_opcode(opcode);
        if (opcode == 0226) {  // SUB A, [HL]
            target = m_bus->read_n8(HL.r16);
        }
        sub<uint8_t>(
            AF.r8.high,
            target,
            intermediate
        );
        AF.r8.high = intermediate;
        break;
    }

    /* SBC A, r8 */
    case 0230 ... 0237:
    {
        uint8_t intermediate, target = *get_r8_from_opcode(opcode);
        if (opcode == 0236) {  // SBC A, [HL]
            target = m_bus->read_n8(HL.r16);
        }
        sub<uint8_t>(
            AF.r8.high,
            target + m_flags.c,
            intermediate
        );
        AF.r8.high = intermediate;
        break;
    }

    /* AND A, r8 */
    case 0240 ... 0247:
    {
        uint8_t intermediate, target = *get_r8_from_opcode(opcode);
        if (opcode == 0246) {  // AND A, [HL]
            target = m_bus->read_n8(HL.r16);
        }
        intermediate = AF.r8.high & target;
        set_flags({
            .z = (intermediate == 0) ? 1 : 0,
            .n = 0,
            .h = 1,
            .c = 0
        });
        AF.r8.high = intermediate;
        break;
    }
    
    /* XOR A, r8 */
    case 0250 ... 0257:
    {
        uint8_t intermediate, target = *get_r8_from_opcode(opcode);
        if (opcode == 0256) {  // XOR A, [HL]
            target = m_bus->read_n8(HL.r16);
        }
        intermediate = AF.r8.high ^ target;
        set_flags({
            .z = (intermediate == 0) ? 1 : 0,
            .n = 0,
            .h = 0,
            .c = 0
        });
        AF.r8.high = intermediate;
        break;
    }

    /* OR A, r8 */
    case 0260 ... 0267:
    {
        uint8_t intermediate, target = *get_r8_from_opcode(opcode);
        if (opcode == 0266) {  // OR A, [HL]
            target = m_bus->read_n8(HL.r16);
        }
        intermediate = AF.r8.high | target;
        set_flags({
            .z = (intermediate == 0) ? 1 : 0,
            .n = 0,
            .h = 0,
            .c = 0
        });
        AF.r8.high = intermediate;
        break;
    }

    /* CP A, r8 */
    case 0270: case 0271: case 0272: case 0273: case 0274: case 0275: case 0276: case 0277:
    {
        uint8_t target = *get_r8_from_opcode(opcode);
        uint8_t intermediate;
        if (opcode == 0276) {  // CP A, [HL]
            target = m_bus->read_n8(HL.r16);
        }
        sub<uint8_t>(
            AF.r8.high,
            target,
            intermediate
        );
        break;
    }

    /* ADD A, n8 */
    case 0306:
        add<uint8_t>(
            AF.r8.high,
            m_bus->read_n8(PC.r16++),
            AF.r8.high
        );
        break;

    /* ADC A, n8 */
    case 0316:
        add<uint8_t>(
            AF.r8.high,
            m_bus->read_n8(PC.r16++) + m_flags.c,
            AF.r8.high
        );
        break;

    /* SUB A, n8 */
    case 0326:
        sub<uint8_t>(
            AF.r8.high,
            m_bus->read_n8(PC.r16++),
            AF.r8.high
        );
        break;

    /* SBC A, n8 */
    case 0336:
        sub<uint8_t>(
            AF.r8.high,
            m_bus->read_n8(PC.r16++) + m_flags.c,
            AF.r8.high
        );
        break;

    /* AND A, n8 */
    case 0346:
    {
        uint8_t intermediate = AF.r8.high & m_bus->read_n8(PC.r16++);
        set_flags({
            .z = (intermediate == 0) ? 1 : 0,
            .n = 0,
            .h = 1,
            .c = 0
        });
        AF.r8.high = intermediate;
        break;
    }

    /* XOR A, n8 */
    case 0356:
    {
        uint8_t intermediate = AF.r8.high ^ m_bus->read_n8(PC.r16++);
        set_flags({
            .z = (intermediate == 0) ? 1 : 0,
            .n = 0,
            .h = 0,
            .c = 0
        });
        AF.r8.high = intermediate;
        break;
    }

    /* OR A, n8 */
    case 0366:
    {
        uint8_t intermediate = AF.r8.high | m_bus->read_n8(PC.r16++);
        set_flags({
            .z = (intermediate == 0) ? 1 : 0,
            .n = 0,
            .h = 0,
            .c = 0
        });
        AF.r8.high = intermediate;
        break;
    }

    /* CP A, n8 */
    case 0376:
    {
        uint8_t target = m_bus->read_n8(PC.r16++);
        uint8_t intermediate;
        sub<uint8_t>(
            AF.r8.high,
            target,
            intermediate
        );
        break;
    }

    /********************************************************************************************
     * 16-bit arithmetic
     ********************************************************************************************/
    
    /* INC BC */
    case 0003:
        add<uint16_t>(
            BC.r16,
            1,
            BC.r16
        );
        break;

    /* INC DE */
    case 0023:
        add<uint16_t>(
            DE.r16,
            1,
            DE.r16
        );
        break;

    /* INC HL */
    case 0043:
        add<uint16_t>(
            HL.r16,
            1,
            HL.r16
        );
        break;

    /* INC SP */
    case 0063:
        add<uint16_t>(
            SP.r16,
            1,
            SP.r16
        );
        break;

    /* DEC BC */
    case 0013:
        sub<uint16_t>(
            BC.r16,
            1,
            BC.r16
        );
        break;

    /* DEC DE */
    case 0033:
        sub<uint16_t>(
            DE.r16,
            1,
            DE.r16
        );
        break;

    /* DEC HL */
    case 0053:
        sub<uint16_t>(
            HL.r16,
            1,
            HL.r16
        );
        break;

    /* DEC SP */
    case 0073:
        sub<uint16_t>(
            SP.r16,
            1,
            SP.r16
        );
        break;

    /* ADD HL, BC */
    case 0011:
        add<uint16_t>(
            HL.r16,
            BC.r16,
            HL.r16
        );
        break;

    /* ADD HL, DE */
    case 0031:
        add<uint16_t>(
            HL.r16,
            DE.r16,
            HL.r16
        );
        break;

    /* ADD HL, HL */
    case 0051:
        add<uint16_t>(
            HL.r16,
            HL.r16,
            HL.r16
        );
        break;

    /* ADD HL, SP */
    case 0071:
        add<uint16_t>(
            HL.r16,
            SP.r16,
            HL.r16
        );
        break;

    /********************************************************************************************
     * 8-bit rotations/shifts
     ********************************************************************************************/

    /* RLCA */
    case 0007:
    {
        uint8_t carry = (AF.r8.high & 0x80) >> 7;  // MSB of A
        AF.r8.high = (AF.r8.high << 1) | carry;
        set_flags({
            .z = 0,
            .n = 0,
            .h = 0,
            .c = carry
        });
        break;
    }

    /* RRCA */
    case 0017:
    {
        uint8_t carry = AF.r8.high & 0x1;  // LSB of A
        AF.r8.high = (AF.r8.high >> 1) | (carry << 7);
        set_flags({
            .z = 0,
            .n = 0,
            .h = 0,
            .c = carry
        });
        break;
    }

    /* RLA */
    case 0027:
    {
        uint8_t carry = (AF.r8.high & 0x80) >> 7;  // MSB of A
        AF.r8.high = (AF.r8.high << 1) | m_flags.c;
        set_flags({
            .z = 0,
            .n = 0,
            .h = 0,
            .c = carry
        });
        break;
    }

    /* RRA */
    case 0037:
    {
        uint8_t carry = AF.r8.high & 0x1;  // LSB of A
        AF.r8.high = (AF.r8.high >> 1) | (m_flags.c << 7);
        set_flags({
            .z = 0,
            .n = 0,
            .h = 0,
            .c = carry
        });
        break;
    }

    /* jumps/calls */
    
    /* JR e8 */
    case 0030:
    {
        int8_t offset = m_bus->read_n8(PC.r16++);  // e8 means 8-bit signed offset
        PC.r16 += offset;
        break;
    }

    /* JR NZ, e8 */
    case 0040:
    {
        int8_t offset = m_bus->read_n8(PC.r16++);
        if (!m_flags.z) {
            PC.r16 += offset;
        }
        break;
    }

    /* JR Z, e8 */
    case 0050:
    {
        int8_t offset = m_bus->read_n8(PC.r16++);
        if (m_flags.z) {
            PC.r16 += offset;
        }
        break;
    }

    /* JR NC, e8 */
    case 0060:
    {
        int8_t offset = m_bus->read_n8(PC.r16++);
        if (!m_flags.c) {
            PC.r16 += offset;
        }
        break;
    }

    /* JR C, e8 */
    case 0070:
    {
        int8_t offset = m_bus->read_n8(PC.r16++);
        if (m_flags.c) {
            PC.r16 += offset;
        }
        break;
    }

    /* RET NZ */
    case 0300:
        if (!m_flags.z) {
            PC.r16 = m_bus->read_n16(SP.r16 += 2);
        }
        break;

    /* RET Z */
    case 0310:
        if (m_flags.z) {
            PC.r16 = m_bus->read_n16(SP.r16 += 2);
        }
        break;

    /* RET NC */
    case 0320:
        if (!m_flags.c) {
            PC.r16 = m_bus->read_n16(SP.r16 += 2);
        }
        break;

    /* RET C */
    case 0330:
        if (m_flags.c) {
            PC.r16 = m_bus->read_n16(SP.r16 += 2);
        }
        break;

    /* RET */
    case 0311:
        PC.r16 = m_bus->read_n16(SP.r16 += 2);
        break;

    /* RETI */
    case 0331:
        m_ime = true;
        PC.r16 = m_bus->read_n16(SP.r16 += 2);
        break;

    /* JP HL */
    case 0351:
        PC.r16 = HL.r16;
        break;

    /* JP NZ, a16 */
    case 0302:
        if (!m_flags.z) {
            PC.r16 = m_bus->read_n16(PC.r16 += 2);
        } else {
            PC.r16 += 2;
        }
        break;

    /* JP Z, a16 */
    case 0312:
        if (m_flags.z) {
            PC.r16 = m_bus->read_n16(PC.r16 += 2);
        } else {
            PC.r16 += 2;
        }
        break;

    /* JP NC, a16 */
    case 0322:
        if (!m_flags.c) {
            PC.r16 = m_bus->read_n16(PC.r16 += 2);
        } else {
            PC.r16 += 2;
        }
        break;

    /* JP C, a16 */
    case 0332:
        if (m_flags.c) {
            PC.r16 = m_bus->read_n16(PC.r16 += 2);
        } else {
            PC.r16 += 2;
        }
        break;

    /* JP a16 */
    case 0303:
        PC.r16 = m_bus->read_n16(PC.r16 += 2);
        break;

    /* CALL NZ, a16 */
    case 0304:
        if (!m_flags.z) {
            m_bus->write_n16(SP.r16 -= 2, PC.r16 + 2);
            PC.r16 = m_bus->read_n16(PC.r16 += 2);
        } else {
            PC.r16 += 2;
        }
        break;

    /* CALL Z, a16 */
    case 0314:
        if (m_flags.z) {
            m_bus->write_n16(SP.r16 -= 2, PC.r16 + 2);
            PC.r16 = m_bus->read_n16(PC.r16 += 2);
        } else {
            PC.r16 += 2;
        }
        break;

    /* CALL NC, a16 */
    case 0324:
        if (!m_flags.c) {
            m_bus->write_n16(SP.r16 -= 2, PC.r16 + 2);
            PC.r16 = m_bus->read_n16(PC.r16 += 2);
        } else {
            PC.r16 += 2;
        }
        break;

    /* CALL C, a16 */
    case 0334:
        if (m_flags.c) {
            m_bus->write_n16(SP.r16 -= 2, PC.r16 + 2);
            PC.r16 = m_bus->read_n16(PC.r16 += 2);
        } else {
            PC.r16 += 2;
        }
        break;

    /* CALL a16 */
    case 0315:
        m_bus->write_n16(SP.r16 -= 2, PC.r16 + 2);
        PC.r16 = m_bus->read_n16(PC.r16 += 2);
        break;

    /* RST 00H */
    case 0307:
        m_bus->write_n16(SP.r16 -= 2, PC.r16);
        PC.r16 = 0x00;
        break;

    /* RST 08H */
    case 0317:
        m_bus->write_n16(SP.r16 -= 2, PC.r16);
        PC.r16 = 0x08;
        break;

    /* RST 10H */
    case 0327:
        m_bus->write_n16(SP.r16 -= 2, PC.r16);
        PC.r16 = 0x10;
        break;

    /* RST 18H */
    case 0337:
        m_bus->write_n16(SP.r16 -= 2, PC.r16);
        PC.r16 = 0x18;
        break;

    /* RST 20H */
    case 0347:
        m_bus->write_n16(SP.r16 -= 2, PC.r16);
        PC.r16 = 0x20;
        break;

    /* RST 28H */
    case 0357:
        m_bus->write_n16(SP.r16 -= 2, PC.r16);
        PC.r16 = 0x28;
        break;

    /* RST 30H */
    case 0367:
        m_bus->write_n16(SP.r16 -= 2, PC.r16);
        PC.r16 = 0x30;
        break;

    /* RST 38H */
    case 0377:
        m_bus->write_n16(SP.r16 -= 2, PC.r16);
        PC.r16 = 0x38;
        break;

    /* misc/control */

    /* NOP */
    case 0000:
        break;

    /* STOP n8 */
    case 0020:
        throw std::runtime_error("STOP opcode not implemented");

    /* HALT */
    case 0166:
        throw std::runtime_error("HALT opcode not implemented");

    /* DI */
    case 0364:
        m_ime = false;
        break;

    /* EI */
    case 0374:
        m_ime = true;
        break;

    /* PREFIX CB */
    case 0xCB00 ... 0xCBFF:
        throw std::runtime_error("CB prefix not implemented");

    default:
        throw std::runtime_error("Received unknown opcode");
    }
}

