#pragma once

#include <cstdint>

class Cartridge;
class PPU;
class WRAM;
class VRAM;
class OAM;
class LCDC;

/* Reference: https://gbdev.io/pandocs/Memory_Map.html */
class Bus {
public:
    Bus() = default;

    uint8_t readImm8(uint16_t address);
    uint16_t readImm16(uint16_t address);
    void writeImm8(uint16_t address, uint8_t data);
    void writeImm16(uint16_t address, uint16_t data);

private:
    Cartridge *m_cartridge = nullptr;
    PPU *m_ppu = nullptr;
    WRAM *m_wram = nullptr;
    VRAM *m_vram = nullptr;
    OAM *m_oam;
};

class VRAM {
public:
    VRAM(Bus *bus) : m_bus(bus) { }

private:
    uint16_t m_data[384] = { 0 };
    Bus *m_bus = nullptr;
};
