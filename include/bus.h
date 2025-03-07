#pragma once

#include <cstdint>
#include <memory>

class Cartridge;
class CPU;
class WRAM;
class VRAM;
class OAM;
class PPU;
class LCDC;

/* Reference: https://gbdev.io/pandocs/Memory_Map.html */
class Bus {
public:
    Bus() = default;

    uint8_t  read_n8(uint16_t address);
    uint16_t read_n16(uint16_t address);
    void     write_n8(uint16_t address, uint8_t data);
    void     write_n16(uint16_t address, uint16_t data);

    void connectCartridge(Cartridge *cartridge);

    /* Gameboy hardware components, will be instantiated if it doesn't exist */
    CPU  *getCPU();
    PPU  *getPPU();
    WRAM *getWRAM();
    VRAM *getVRAM();
    OAM  *getOAM();
    LCDC *getLCDC();

private:
    std::unique_ptr<Cartridge> m_cartridge;
    std::unique_ptr<CPU>       m_cpu;
    std::unique_ptr<PPU>       m_ppu;
    std::unique_ptr<WRAM>      m_wram;
    std::unique_ptr<VRAM>      m_vram;
    std::unique_ptr<OAM>       m_oam;
    std::unique_ptr<LCDC>      m_lcdc;
};
