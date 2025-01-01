#include <stdexcept>
#include "cpu.h"

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

void CPU::set_flags(CPUFlags flags)
{
    uint8_t new_flags = AF & 0xFF;
    if (flags.z >= 0) new_flags = (new_flags & ~0x80) | (flags.z << 7);
    if (flags.n >= 0) new_flags = (new_flags & ~0x40) | (flags.n << 6);
    if (flags.h >= 0) new_flags = (new_flags & ~0x20) | (flags.h << 5);
    if (flags.c >= 0) new_flags = (new_flags & ~0x10) | (flags.c << 4);
    AF = (AF & 0xFF00) | new_flags;
}

void CPU::cycle()
{
    uint8_t opcode = m_bus->readImm8(PC++);
    switch (opcode) {
        case 0:  // nop
            break;

        /* ld r16, imm16 */
        case 0b00000001:  // ld bc, imm16
            BC = m_bus->readImm16(PC += 2);
            break;
        case 0b00010001:  // ld de, imm16
            DE = m_bus->readImm16(PC += 2);
            break;
        case 0b00100001:  // ld hl, imm16
            HL = m_bus->readImm16(PC += 2);
            break;
        case 0b00110001: {  // ld sp, imm16
            SP = m_bus->readImm16(PC += 2);
            break;
        }

        /* ld [r16mem], a */
        case 0b00000010:  // ld [bc], a
            m_bus->writeImm8(BC, AF & 0xFF00);
            break;
        case 0b00010010:  // ld [de], a
            m_bus->writeImm8(DE, AF & 0xFF00);
            break;
        case 0b00100010:  // ld [hl+], a
            m_bus->writeImm8(HL, AF & 0xFF00);
            HL++;
            break;
        case 0b00110010:  // ld [hl-], a
            m_bus->writeImm8(HL, AF & 0xFF00);
            HL--;
            break;

        /* ld a, [r16mem] */
        case 0b00001010:  // ld a, [bc]
            AF = (AF & 0x00FF) | (m_bus->readImm8(BC) << 8);
            break;
        case 0b00011010:  // ld a, [de]
            AF = (AF & 0x00FF) | (m_bus->readImm8(DE) << 8);
            break;
        case 0b00101010:  // ld a, [hl+]
            AF = (AF & 0x00FF) | (m_bus->readImm8(HL) << 8);
            HL++;
            break;
        case 0b00111010:  // ld a, [hl-]
            AF = (AF & 0x00FF) | (m_bus->readImm8(HL) << 8);
            HL--;
            break;

        /* ld [imm16], sp */
        case 0b00001000:
            m_bus->writeImm16(m_bus->readImm16(PC += 2), SP);
            break;

        /* inc r16 */
        case 0b00000011:  // inc bc
            add<uint16_t>(BC, 1, BC, true);
            break;
        case 0b00010011:  // inc de
            add<uint16_t>(DE, 1, DE, true);
            break;
        case 0b00100011:  // inc hl
            add<uint16_t>(HL, 1, HL, true);
            break;
        case 0b00110011:  // inc sp
            add<uint16_t>(SP, 1, SP, true);
            break;

        /* dec r16 */
        case 0b00001011:
            sub<uint16_t>(BC, 1, BC, true);
            break;
        case 0b00011011:
            sub<uint16_t>(DE, 1, DE, true);
            break;
        case 0b00101011:
            sub<uint16_t>(HL, 1, HL, true);
            break;
        case 0b00111011:
            sub<uint16_t>(SP, 1, SP, true);
            break;

        /* add hl, r16 */
        case 0b00001001:
            add<uint16_t>(HL, BC, HL, true);
            break;
        case 0b00011001:
            add<uint16_t>(HL, DE, HL, true);
            break;
        case 0b00101001:
            add<uint16_t>(HL, HL, HL, true);
            break;
        case 0b00111001:
            add<uint16_t>(HL, SP, HL, true);
            break;

        /* inc r8 */
        {
            uint8_t int_result;
            case 0b00000100:  // inc b
                add<uint8_t>(BC >> 8, 1, int_result, false);
                BC = (int_result << 8) | (BC & 0x00FF);
                break;
            case 0b00001100:  // inc c
                add<uint8_t>(BC & 0x00FF, 1, int_result, false);
                BC = int_result | (BC & 0xFF00);
                break;
            case 0b00010100:  // inc d
                add<uint8_t>(DE >> 8, 1, int_result, false);
                DE = (int_result << 8) | (DE & 0x00FF);
                break;
            case 0b00011100:  // inc e
                add<uint8_t>(DE & 0x00FF, 1, int_result, false);
                DE = int_result | (DE & 0xFF00);
                break;
            case 0b00100100:  // inc h
                add<uint8_t>(HL >> 8, 1, int_result, false);
                HL = (int_result << 8) | (HL & 0x00FF);
                break;
            case 0b00101100:  // inc l
                add<uint8_t>(HL & 0x00FF, 1, int_result, false);
                HL = int_result | (HL & 0xFF00);
                break;
            case 0b00110100:  // inc [hl]
                int_result = m_bus->readImm8(HL);
                add<uint8_t>(int_result, 1, int_result, false);
                m_bus->writeImm8(HL, int_result);
                break;
            case 0b00111100:  // inc a
                add<uint8_t>(AF >> 8, 1, int_result, false);
                AF = (int_result << 8) | (AF & 0x00FF);
                break;
        }

        default:
            throw std::runtime_error("Received unknown opcode");
    }
}
