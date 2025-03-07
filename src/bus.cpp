#include "bus.h"

uint8_t Bus::read_n8(uint16_t address)
{
    return 0;
}

uint16_t Bus::read_n16(uint16_t address)
{
    uint8_t u = read_n8(address), l = read_n8(address + 1);
    return (uint16_t)l | ((uint16_t)u << 8);
}

void Bus::write_n8(uint16_t address, uint8_t data)
{
}

void Bus::writeImm16(uint16_t address, uint8_t data)
{
}
