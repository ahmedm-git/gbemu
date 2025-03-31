#pragma once

#include <cstdint>
#include <memory>
#include "rom.h"

/* Reference: https://gbdev.io/pandocs/Memory_Map.html */
class Bus {
public:
    Bus();
    Bus(ROM *cartridge) : m_cartridge(cartridge) {}

    uint8_t  read_n8(uint16_t address);
    uint16_t read_n16(uint16_t address);
    void     write_n8(uint16_t address, uint8_t data);
    void     write_n16(uint16_t address, uint16_t data);

    ROM      *get_cartridge();
//    IO       *get_io();
//    OAM      *get_oam();
//    PPU      *get_ppu();
//    VRAM     *get_vram();
//    WRAM     *get_wram();
//    HRAM     *get_hram();

private:
    std::unique_ptr<ROM>       m_cartridge;
//    std::unique_ptr<PPU>       m_ppu;
//    std::unique_ptr<WRAM>      m_wram;
//    std::unique_ptr<VRAM>      m_vram;
//    std::unique_ptr<OAM>       m_oam;
//    std::unique_ptr<IO>        m_io;
//    std::unique_ptr<HRAM>      m_hram;
};
