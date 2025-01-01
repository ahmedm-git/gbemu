#include "bus.h"

uint8_t Bus::readImm8(uint16_t address)
{
    return 0;
}

uint16_t Bus::readImm16(uint16_t address)
{
    uint8_t u = readImm8(address), l = readImm8(address + 1);
    return (uint16_t)l | ((uint16_t)u << 8);
}

void Bus::writeImm8(uint16_t address, uint8_t data)
{
}

void Bus::writeImm16(uint16_t address, uint8_t data)
{
}
