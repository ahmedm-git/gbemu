#include <iostream>
#include "cpu.h"

#define DEBUG

/*
 * reset() - Resets the CPU registers and flags to their initial state.
 */
void CPU::reset() {
    m_state.AF.r16 = 0x0000;
    m_state.BC.r16 = 0x0000;
    m_state.DE.r16 = 0x0000;
    m_state.HL.r16 = 0x0000;
    m_state.SP.r16 = 0x0000;
    m_state.PC.r16 = 0x0000;
    m_state.FLAGS.flags = 0b00000000;
    m_state.IME = 0;
    m_state.IF = 0;
    m_state.IE = 0;
    m_state.MCYCLES = 0;
}

/*
 * fetch() - Fetches the next byte from memory at the current program counter (PC) address.
 * 
 * Returns:
 *   uint8_t: The fetched byte from memory.
 */
uint8_t CPU::fetch() {
    return m_bus->read_n8(m_state.PC.r16++);
}

/*
 * step() - Executes a single CPU instruction.
 */
void CPU::step() {
    if (m_cycles_to_wait > 0) {
        m_cycles_to_wait--;
        m_state.MCYCLES++;
        return;
    }

    uint8_t opcode = fetch();
    uint8_t cycle_count = 0;

    switch (opcode) {
        /********************************************************************************************
         * 8-bit loads
         ********************************************************************************************/

        /* LD r8, r8 */
        case 0100 ... 0165: case 0167 ... 0177:
            #ifdef DEBUG
            std::cout << "LD r8, r8" << std::endl;
            #endif
            if ((opcode % 010) == 06) {
                m_bus->write_n8(m_state.HL.r16, *get_r8_from_opcode(opcode));
            } else if ((opcode >> 4) == 016) {
                *get_r8_from_opcode(opcode) = m_bus->read_n8(m_state.HL.r16);
            } else {
                *get_r8_from_opcode(opcode) = *get_r8_from_opcode(opcode, 3);
            }
            cycle_count = 1;
            break;

        /* LD [r16], A */
        case 0002: case 0022: case 0042: case 0062:
            #ifdef DEBUG
            std::cout << "LD [r16], A" << std::endl;
            #endif
            m_bus->write_n8(get_r16_from_opcode(opcode, 3)->r16, m_state.AF.r8.hi);
            if (opcode == 062) {
                m_state.HL.r16--;
            } else if (opcode == 042) {
                m_state.HL.r16++;
            }
            cycle_count = 1;
            break;

        /* LD A, [r16] */
        case 0012: case 0032: case 0052: case 0072:
            m_state.AF.r8.hi = m_bus->read_n8(get_r16_from_opcode(opcode)->r16);
            if (opcode == 072) {
                m_state.HL.r16--;
            } else if (opcode == 052) {
                m_state.HL.r16++;
            }
            cycle_count = 1;
            break;

        /* LD r8, n8 */
        case 0006: case 0016: case 0026: case 0036: case 0046: case 0056: case 0066: case 0076:
            *get_r8_from_opcode(opcode, 0) = m_bus->read_n8(m_state.PC.r16++);
            if (opcode == 0066) {  // LD [m_state.HL], n8
                m_bus->write_n8(m_state.HL.r16, *get_r8_from_opcode(opcode, 0));
            }
            cycle_count = 2;
            break;

        /* LDH [a8], A */
        case 0340:
            m_bus->write_n8(0xFF00 + m_bus->read_n8(m_state.PC.r16++), m_state.AF.r8.hi);
            cycle_count = 2;
            break;

        /* LDH A, [a8] */
        case 0360:
            m_state.AF.r8.hi = m_bus->read_n8(0xFF00 + m_bus->read_n8(m_state.PC.r16++));
            cycle_count = 2;
            break;

        /* LDH [C], A */
        case 0342:
            m_bus->write_n8(0xFF00 + m_state.BC.r8.lo, m_state.AF.r8.hi);
            cycle_count = 1;
            break;

        /* LD [a16], A */
        case 0352:
            m_bus->write_n8(m_bus->read_n16(m_state.PC.r16 += 2), m_state.AF.r8.hi);
            cycle_count = 3;
            break;

        /* LDH A, [C] */
        case 0362:
            m_state.AF.r8.hi = m_bus->read_n8(0xFF00 + m_state.BC.r8.lo);
            cycle_count = 1;
            break;

        /* LD A, [a16] */
        case 0372:
            m_state.AF.r8.hi = m_bus->read_n8(m_bus->read_n16(m_state.PC.r16 += 2));
            cycle_count = 3;
            break;

        /********************************************************************************************
         * 16-bit loads
         ********************************************************************************************/

        /* LD BC, n16 */
        case 0001:
            m_state.BC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
            cycle_count = 3;
            break;

        /* LD DE, n16 */
        case 0021:
            m_state.DE.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
            cycle_count = 3;
            break;

        /* LD HL, n16 */
        case 0041:
            m_state.HL.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
            cycle_count = 3;
            break;

        /* LD SP, n16 */
        case 0061:
            m_state.SP.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
            cycle_count = 3;
            break;

        /* LD [a16], SP */
        case 0010:
            m_bus->write_n16(m_bus->read_n16(m_state.PC.r16 += 2), m_state.SP.r16);
            cycle_count = 5;
            break;

        /* LD HL, SP+n8 */
        case 0370:
            m_state.HL.r16 = m_state.SP.r16 + m_bus->read_n8(m_state.PC.r16++);
            cycle_count = 3;
            break;

        /* POP BC */
        case 0301:
            m_state.BC.r16 = m_bus->read_n16(m_state.SP.r16 += 2);
            cycle_count = 3;
            break;

        /* POP DE */
        case 0321:
            m_state.DE.r16 = m_bus->read_n16(m_state.SP.r16 += 2);
            cycle_count = 3;
            break;

        /* POP HL */
        case 0341:
            m_state.HL.r16 = m_bus->read_n16(m_state.SP.r16 += 2);
            cycle_count = 3;
            break;

        /* POP AF */
        case 0361:
            m_state.AF.r16 = m_bus->read_n16(m_state.SP.r16 += 2);
            cycle_count = 3;
            break;

        /* LD SP, HL */
        case 0371:
            m_state.SP.r16 = m_state.HL.r16;
            cycle_count = 2;
            break;

        /* PUSH BC */
        case 0305:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.BC.r16);
            cycle_count = 4;
            break;

        /* PUSH DE */
        case 0325:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.DE.r16);
            cycle_count = 4;
            break;

        /* PUSH HL */
        case 0345:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.HL.r16);
            cycle_count = 4;
            break;

        /* PUSH AF */
        case 0365:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.AF.r16);
            cycle_count = 4;
            break;

        /********************************************************************************************
         * 8-bit arithmetic
         ********************************************************************************************/

        /* INC r8 */
        case 0004: case 0014: case 0024: case 0034: case 0044: case 0054: case 0064: case 0074:
            {
                uint8_t intermediate, target = *get_r8_from_opcode(opcode, 3);
                if (opcode == 0064) {  // INC [m_state.HL]
                    target = m_bus->read_n8(m_state.HL.r16);
                }
                add<uint8_t>(
                    target,
                    1,
                    intermediate
                );
                if (opcode == 0064) {  // INC [m_state.HL]
                    m_bus->write_n8(m_state.HL.r16, intermediate);
                } else {
                    *get_r8_from_opcode(opcode, 3) = intermediate;
                }
                cycle_count = 1;
                break;
            }

        /* DEC r8 */
        case 0005: case 0015: case 0025: case 0035: case 0045: case 0055: case 0065: case 0075:
            {
                uint8_t intermediate, target = *get_r8_from_opcode(opcode, 3);
                if (opcode == 0065) {  // DEC [m_state.HL]
                    target = m_bus->read_n8(m_state.HL.r16);
                }
                sub<uint8_t>(
                    target,
                    1,
                    intermediate
                );
                if (opcode == 0065) {  // DEC [m_state.HL]
                    m_bus->write_n8(m_state.HL.r16, intermediate);
                } else {
                    *get_r8_from_opcode(opcode, 3) = intermediate;
                }
                cycle_count = 1;
                break;
            }

        /* DAA */
        case 0047:
            {
                uint8_t adjustment = 0;
                if (m_state.FLAGS.bits.n) {
                    if (m_state.FLAGS.bits.h) {
                        adjustment += 0x6;
                    }
                    if (m_state.FLAGS.bits.c) {
                        adjustment += 0x60;
                    }
                    m_state.AF.r8.hi -= adjustment;
                } else {
                    if (m_state.FLAGS.bits.h || (m_state.AF.r8.hi & 0xF) > 0x9) {
                        adjustment += 0x6;
                    }
                    if (m_state.FLAGS.bits.c || m_state.AF.r8.hi > 0x99) {
                        adjustment += 0x60;
                        m_state.FLAGS.bits.c = 1;
                    }
                    m_state.AF.r8.hi += adjustment;
                }
                cycle_count = 1;
                break;
            }

        /* CPL (bitwise not for A) */
        case 0057:
            m_state.AF.r8.hi = ~m_state.AF.r8.hi;
            cycle_count = 1;
            break;

        /* SCF (set carry flag) */
        case 0067:
            m_state.FLAGS.bits = {
                .c = 1,
                .h = 0,
                .n = 0,
                .z = m_state.FLAGS.bits.z
            };
            cycle_count = 1;
            break;

        /* CCF (complement carry flag) */
        case 0077:
            m_state.FLAGS.bits = {
                .c = !m_state.FLAGS.bits.c,
                .h = 0,
                .n = 0,
                .z = m_state.FLAGS.bits.z
            };
            cycle_count = 1;
            break;

        /* ADD A, r8 */
        case 0200 ... 0207:
            {
                uint8_t intermediate, target = *get_r8_from_opcode(opcode);
                if (opcode == 0206) {  // ADD A, [m_state.HL]
                    target = m_bus->read_n8(m_state.HL.r16);
                }
                add<uint8_t>(
                    m_state.AF.r8.hi,
                    target,
                    intermediate
                );
                m_state.AF.r8.hi = intermediate;
                cycle_count = 1;
                break;
            }

        /* ADC A, r8 */
        case 0210 ... 0217:
            {
                uint8_t intermediate, target = *get_r8_from_opcode(opcode);
                if (opcode == 0216) {  // ADC A, [m_state.HL]
                    target = m_bus->read_n8(m_state.HL.r16);
                }
                add<uint8_t>(
                    m_state.AF.r8.hi,
                    target + m_state.FLAGS.bits.c,
                    intermediate
                );
                m_state.AF.r8.hi = intermediate;
                cycle_count = 1;
                break;
            }

        /* SUB A, r8 */
        case 0220 ... 0227:
            {
                uint8_t intermediate, target = *get_r8_from_opcode(opcode);
                if (opcode == 0226) {  // SUB A, [m_state.HL]
                    target = m_bus->read_n8(m_state.HL.r16);
                }
                sub<uint8_t>(
                    m_state.AF.r8.hi,
                    target,
                    intermediate
                );
                m_state.AF.r8.hi = intermediate;
                cycle_count = 1;
                break;
            }

        /* SBC A, r8 */
        case 0230 ... 0237:
            {
                uint8_t intermediate, target = *get_r8_from_opcode(opcode);
                if (opcode == 0236) {  // Sm_state.BC A, [m_state.HL]
                    target = m_bus->read_n8(m_state.HL.r16);
                }
                sub<uint8_t>(
                    m_state.AF.r8.hi,
                    target + m_state.FLAGS.bits.c,
                    intermediate
                );
                m_state.AF.r8.hi = intermediate;
                cycle_count = 1;
                break;
            }

        /* AND A, r8 */
        case 0240 ... 0247:
            {
                uint8_t intermediate, target = *get_r8_from_opcode(opcode);
                if (opcode == 0246) {  // AND A, [m_state.HL]
                    target = m_bus->read_n8(m_state.HL.r16);
                }
                intermediate = m_state.AF.r8.hi & target;
                m_state.FLAGS.bits = {
                    .c = 0,
                    .h = 1,
                    .n = 0,
                    .z = (uint8_t)((intermediate == 0) ? 1 : 0)
                };
                m_state.AF.r8.hi = intermediate;
                cycle_count = 1;
                break;
            }

        /* XOR A, r8 */
        case 0250 ... 0257:
            {
                uint8_t intermediate, target = *get_r8_from_opcode(opcode);
                if (opcode == 0256) {  // XOR A, [m_state.HL]
                    target = m_bus->read_n8(m_state.HL.r16);
                }
                intermediate = m_state.AF.r8.hi ^ target;
                m_state.FLAGS.bits = {
                    .c = 0,
                    .h = 0,
                    .n = 0,
                    .z = (uint8_t)((intermediate == 0) ? 1 : 0)
                };
                m_state.AF.r8.hi = intermediate;
                cycle_count = 1;
                break;
            }

        /* OR A, r8 */
        case 0260 ... 0267:
            {
                uint8_t intermediate, target = *get_r8_from_opcode(opcode);
                if (opcode == 0266) {  // OR A, [m_state.HL]
                    target = m_bus->read_n8(m_state.HL.r16);
                }
                intermediate = m_state.AF.r8.hi | target;
                m_state.FLAGS.bits = {
                    .c = 0,
                    .h = 0,
                    .n = 0,
                    .z = (uint8_t)((intermediate == 0) ? 1 : 0)
                };
                m_state.AF.r8.hi = intermediate;
                cycle_count = 1;
                break;
            }

        /* CP A, r8 */
        case 0270: case 0271: case 0272: case 0273: case 0274: case 0275: case 0276: case 0277:
            {
                uint8_t target = *get_r8_from_opcode(opcode);
                uint8_t intermediate;
                if (opcode == 0276) {  // CP A, [HL]
                    target = m_bus->read_n8(m_state.HL.r16);
                }
                sub<uint8_t>(
                    m_state.AF.r8.hi,
                    target,
                    intermediate
                );
                cycle_count = 1;
                break;
            }

        /* ADD A, n8 */
        case 0306:
            add<uint8_t>(
                m_state.AF.r8.hi,
                m_bus->read_n8(m_state.PC.r16++),
                m_state.AF.r8.hi
            );
            cycle_count = 2;
            break;

        /* ADC A, n8 */
        case 0316:
            add<uint8_t>(
                m_state.AF.r8.hi,
                m_bus->read_n8(m_state.PC.r16++) + m_state.FLAGS.bits.c,
                m_state.AF.r8.hi
            );
            cycle_count = 2;
            break;

        /* SUB A, n8 */
        case 0326:
            sub<uint8_t>(
                m_state.AF.r8.hi,
                m_bus->read_n8(m_state.PC.r16++),
                m_state.AF.r8.hi
            );
            cycle_count = 2;
            break;

        /* SBC A, n8 */
        case 0336:
            sub<uint8_t>(
                m_state.AF.r8.hi,
                m_bus->read_n8(m_state.PC.r16++) + m_state.FLAGS.bits.c,
                m_state.AF.r8.hi
            );
            cycle_count = 2;
            break;

        /* AND A, n8 */
        case 0346:
            {
                uint8_t intermediate = m_state.AF.r8.hi & m_bus->read_n8(m_state.PC.r16++);
                m_state.FLAGS.bits = {
                    .c = 0,
                    .h = 1,
                    .n = 0,
                    .z = (uint8_t)((intermediate == 0) ? 1 : 0)
                };
                m_state.AF.r8.hi = intermediate;
                cycle_count = 2;
                break;
            }

        /* XOR A, n8 */
        case 0356:
            {
                uint8_t intermediate = m_state.AF.r8.hi ^ m_bus->read_n8(m_state.PC.r16++);
                m_state.FLAGS.bits = {
                    .c = 0,
                    .h = 0,
                    .n = 0,
                    .z = (uint8_t)((intermediate == 0) ? 1 : 0)
                };
                m_state.AF.r8.hi = intermediate;
                cycle_count = 2;
                break;
            }

        /* OR A, n8 */
        case 0366:
            {
                uint8_t intermediate = m_state.AF.r8.hi | m_bus->read_n8(m_state.PC.r16++);
                m_state.FLAGS.bits = {
                    .c = 0,
                    .h = 0,
                    .n = 0,
                    .z = (uint8_t)((intermediate == 0) ? 1 : 0)
                };
                m_state.AF.r8.hi = intermediate;
                cycle_count = 2;
                break;
            }

        /* CP A, n8 */
        case 0376:
            {
                uint8_t target = m_bus->read_n8(m_state.PC.r16++);
                uint8_t intermediate;
                sub<uint8_t>(
                    m_state.AF.r8.hi,
                    target,
                    intermediate
                );
                cycle_count = 2;
                break;
            }

        /********************************************************************************************
         * 16-bit arithmetic
         ********************************************************************************************/

        /* INC BC */
        case 0003:
            add<uint16_t>(
                m_state.BC.r16,
                1,
                m_state.BC.r16
            );
            cycle_count = 2;
            break;

        /* INC DE */
        case 0023:
            add<uint16_t>(
                m_state.DE.r16,
                1,
                m_state.DE.r16
            );
            cycle_count = 2;
            break;

        /* INC HL */
        case 0043:
            add<uint16_t>(
                m_state.HL.r16,
                1,
                m_state.HL.r16
            );
            cycle_count = 2;
            break;

        /* INC SP */
        case 0063:
            add<uint16_t>(
                m_state.SP.r16,
                1,
                m_state.SP.r16
            );
            cycle_count = 2;
            break;

        /* DEC BC */
        case 0013:
            sub<uint16_t>(
                m_state.BC.r16,
                1,
                m_state.BC.r16
            );
            cycle_count = 2;
            break;

        /* DEC DE */
        case 0033:
            sub<uint16_t>(
                m_state.DE.r16,
                1,
                m_state.DE.r16
            );
            cycle_count = 2;
            break;

        /* DEC HL */
        case 0053:
            sub<uint16_t>(
                m_state.HL.r16,
                1,
                m_state.HL.r16
            );
            cycle_count = 2;
            break;

        /* DEC SP */
        case 0073:
            sub<uint16_t>(
                m_state.SP.r16,
                1,
                m_state.SP.r16
            );
            cycle_count = 2;
            break;

        /* ADD HL, BC */
        case 0011:
            add<uint16_t>(
                m_state.HL.r16,
                m_state.BC.r16,
                m_state.HL.r16
            );
            cycle_count = 2;
            break;

        /* ADD HL, DE */
        case 0031:
            add<uint16_t>(
                m_state.HL.r16,
                m_state.DE.r16,
                m_state.HL.r16
            );
            cycle_count = 2;
            break;

        /* ADD HL, HL */
        case 0051:
            add<uint16_t>(
                m_state.HL.r16,
                m_state.HL.r16,
                m_state.HL.r16
            );
            cycle_count = 2;
            break;

        /* ADD HL, SP */
        case 0071:
            add<uint16_t>(
                m_state.HL.r16,
                m_state.SP.r16,
                m_state.HL.r16
            );
            cycle_count = 2;
            break;

        /********************************************************************************************
         * 8-bit rotations/shifts
         ********************************************************************************************/

        /* RLCA */
        case 0007:
            {
                uint8_t carry = (m_state.AF.r8.hi & 0x80) >> 7;  // MSB of A
                m_state.AF.r8.hi = (m_state.AF.r8.hi << 1) | carry;
                m_state.FLAGS.bits = {
                    .c = carry,
                    .h = 0,
                    .n = 0,
                    .z = 0
                };
                cycle_count = 1;
                break;
            }

        /* RRCA */
        case 0017:
            {
                uint8_t carry = m_state.AF.r8.hi & 0x1;  // LSB of A
                m_state.AF.r8.hi = (m_state.AF.r8.hi >> 1) | (carry << 7);
                m_state.FLAGS.bits = {
                    .c = carry,
                    .h = 0,
                    .n = 0,
                    .z = 0
                };
                cycle_count = 1;
                break;
            }

        /* RLA */
        case 0027:
            {
                uint8_t carry = (m_state.AF.r8.hi & 0x80) >> 7;  // MSB of A
                m_state.AF.r8.hi = (m_state.AF.r8.hi << 1) | m_state.FLAGS.bits.c;
                m_state.FLAGS.bits = {
                    .c = carry,
                    .h = 0,
                    .n = 0,
                    .z = 0
                };
                break;
            }

        /* RRA */
        case 0037:
            {
                uint8_t carry = m_state.AF.r8.hi & 0x1;  // LSB of A
                m_state.AF.r8.hi = (m_state.AF.r8.hi >> 1) | (m_state.FLAGS.bits.c << 7);
                m_state.FLAGS.bits = {
                    .c = carry,
                    .h = 0,
                    .n = 0,
                    .z = 0
                };
                cycle_count = 1;
                break;
            }

        /* jumps/calls */

        /* JR e8 */
        case 0030:
            {
                int8_t offset = (int8_t)m_bus->read_n16(m_state.PC.r16++);
                m_state.PC.r16 += offset;
                cycle_count = 3;
                break;
            }

        /* JR NZ, e8 */
        case 0040:
            {
                int8_t offset = m_bus->read_n8(m_state.PC.r16++);
                if (!m_state.FLAGS.bits.z) {
                    m_state.PC.r16 += offset;
                    cycle_count = 3;
                } else {
                    cycle_count = 2;
                }
                break;
            }

        /* JR Z, e8 */
        case 0050:
            {
                int8_t offset = m_bus->read_n8(m_state.PC.r16++);
                if (m_state.FLAGS.bits.z) {
                    m_state.PC.r16 += offset;
                    cycle_count = 3;
                } else {
                    cycle_count = 2;
                }
                break;
            }

        /* JR NC, e8 */
        case 0060:
            {
                int8_t offset = m_bus->read_n8(m_state.PC.r16++);
                if (!m_state.FLAGS.bits.c) {
                    m_state.PC.r16 += offset;
                    cycle_count = 3;
                } else {
                    cycle_count = 2;
                }
                break;
            }

        /* JR C, e8 */
        case 0070:
            {
                int8_t offset = m_bus->read_n8(m_state.PC.r16++);
                if (m_state.FLAGS.bits.c) {
                    m_state.PC.r16 += offset;
                    cycle_count = 3;
                } else {
                    cycle_count = 2;
                }
                break;
            }

        /* RET NZ */
        case 0300:
            if (!m_state.FLAGS.bits.z) {
                m_state.PC.r16 = m_bus->read_n16(m_state.SP.r16 += 2);
                cycle_count = 5;
            } else {
                cycle_count = 2;
            }
            break;

        /* RET Z */
        case 0310:
            if (m_state.FLAGS.bits.z) {
                m_state.PC.r16 = m_bus->read_n16(m_state.SP.r16 += 2);
                cycle_count = 5;
            } else {
                cycle_count = 2;
            }
            break;

        /* RET NC */
        case 0320:
            if (!m_state.FLAGS.bits.c) {
                m_state.PC.r16 = m_bus->read_n16(m_state.SP.r16 += 2);
                cycle_count = 5;
            } else {
                cycle_count = 2;
            }
            break;

        /* RET C */
        case 0330:
            if (m_state.FLAGS.bits.c) {
                m_state.PC.r16 = m_bus->read_n16(m_state.SP.r16 += 2);
                cycle_count = 5;
            } else {
                cycle_count = 2;
            }
            break;

        /* RET */
        case 0311:
            m_state.PC.r16 = m_bus->read_n16(m_state.SP.r16 += 2);
            cycle_count = 4;
            break;

        /* RETI */
        case 0331:
            m_state.IME = true;
            m_state.PC.r16 = m_bus->read_n16(m_state.SP.r16 += 2);
            cycle_count = 4;
            break;

        /* JP HL */
        case 0351:
            m_state.PC.r16 = m_state.HL.r16;
            cycle_count = 1;
            break;

        /* JP NZ, a16 */
        case 0302:
            if (!m_state.FLAGS.bits.z) {
                m_state.PC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
                cycle_count = 3;
            } else {
                m_state.PC.r16 += 2;
                cycle_count = 2;
            }
            break;

        /* JP Z, a16 */
        case 0312:
            if (m_state.FLAGS.bits.z) {
                m_state.PC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
                cycle_count = 3;
            } else {
                m_state.PC.r16 += 2;
                cycle_count = 2;
            }
            break;

        /* JP NC, a16 */
        case 0322:
            if (!m_state.FLAGS.bits.c) {
                m_state.PC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
                cycle_count = 3;
            } else {
                m_state.PC.r16 += 2;
                cycle_count = 2;
            }
            break;

        /* JP C, a16 */
        case 0332:
            if (m_state.FLAGS.bits.c) {
                m_state.PC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
                cycle_count = 3;
            } else {
                m_state.PC.r16 += 2;
                cycle_count = 2;
            }
            break;

        /* JP a16 */
        case 0303:
            #ifdef DEBUG
            std::cout << "JP a16" << std::endl;
            #endif
            m_state.PC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
            cycle_count = 4;
            break;

        /* CALL NZ, a16 */
        case 0304:
            if (!m_state.FLAGS.bits.z) {
                m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16 + 2);
                m_state.PC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
                cycle_count = 6;
            } else {
                m_state.PC.r16 += 2;
                cycle_count = 3;
            }
            break;

        /* CALL Z, a16 */
        case 0314:
            if (m_state.FLAGS.bits.z) {
                m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16 + 2);
                m_state.PC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
                cycle_count = 6;
            } else {
                m_state.PC.r16 += 2;
                cycle_count = 3;
            }
            break;

        /* CALL NC, a16 */
        case 0324:
            if (!m_state.FLAGS.bits.c) {
                m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16 + 2);
                m_state.PC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
                cycle_count = 6;
            } else {
                m_state.PC.r16 += 2;
                cycle_count = 3;
            }
            break;

        /* CALL C, a16 */
        case 0334:
            if (m_state.FLAGS.bits.c) {
                m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16 + 2);
                m_state.PC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
                cycle_count = 6;
            } else {
                m_state.PC.r16 += 2;
                cycle_count = 3;
            }
            break;

        /* CALL a16 */
        case 0315:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16 + 2);
            m_state.PC.r16 = m_bus->read_n16(m_state.PC.r16 += 2);
            cycle_count = 6;
            break;

        /* RST 00H */
        case 0307:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16);
            m_state.PC.r16 = 0x00;
            cycle_count = 4;
            break;

        /* RST 08H */
        case 0317:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16);
            m_state.PC.r16 = 0x08;
            cycle_count = 4;
            break;

        /* RST 10H */
        case 0327:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16);
            m_state.PC.r16 = 0x10;
            cycle_count = 4;
            break;

        /* RST 18H */
        case 0337:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16);
            m_state.PC.r16 = 0x18;
            cycle_count = 4;
            break;

        /* RST 20H */
        case 0347:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16);
            m_state.PC.r16 = 0x20;
            cycle_count = 4;
            break;

        /* RST 28H */
        case 0357:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16);
            m_state.PC.r16 = 0x28;
            cycle_count = 4;
            break;

        /* RST 30H */
        case 0367:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16);
            m_state.PC.r16 = 0x30;
            cycle_count = 4;
            break;

        /* RST 38H */
        case 0377:
            m_bus->write_n16(m_state.SP.r16 -= 2, m_state.PC.r16);
            m_state.PC.r16 = 0x38;
            cycle_count = 4;
            break;

        /* misc/control */

        /* NOP */
        case 0000:
            cycle_count = 1;
            break;

        /* STOP n8 */
        case 0020:
            throw std::runtime_error("STOP opcode not implemented");

        /* HALT */
        case 0166:
            throw std::runtime_error("HALT opcode not implemented");

        /* DI */
        case 0364:
            m_state.IME = false;
            cycle_count = 1;
            break;

        /* EI */
        case 0374:
            m_state.IME = true;
            cycle_count = 1;
            break;

        /* PREFIX CB */
        case 0xCB00 ... 0xCBFF:
            throw std::runtime_error("CB prefix not implemented");

        default:
            throw std::runtime_error("Received unknown opcode");
    }

    m_cycles_to_wait = cycle_count;
    m_state.MCYCLES += cycle_count;
}
