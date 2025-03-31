#include "bus.h"

Bus::Bus() {
    m_cartridge = std::make_unique<ROM>();
    // m_vram = std::make_unique<VRAM>();
    // m_wram = std::make_unique<WRAM>();
    // m_oam = std::make_unique<OAM>();
    // m_io = std::make_unique<IO>();
    // m_hram = std::make_unique<HRAM>();
}

uint8_t Bus::read_n8(uint16_t address) {
    if (address < 0x8000) {
        // ROM
        return m_cartridge->read_n8(address);
    } // else if (address < 0xA000) {
//        // VRAM
//        return m_vram->read_n8(address);
//    } else if (address < 0xC000) {
//        // External RAM
//        return m_wram->read_n8(address);
//    } else if (address < 0xE000) {
//        // WRAM
//        return m_wram->read_n8(address);
//    } else if (address < 0xFE00) {
//        // Echo RAM
//        return m_wram->read_n8(address);
//    } else if (address < 0xFF00) {
//        // OAM
//        return m_oam->read_n8(address);
//    } else if (address < 0xFF80) {
//        // IO Registers
//        return m_io->read_n8(address);
//    } else {
//        // HRAM
//        return m_wram->read_n8(address);
//    }
}

uint16_t Bus::read_n16(uint16_t address) {
    if (address < 0x8000) {
        // ROM
        return m_cartridge->read_n16(address);
    } // else if (address < 0xA000) {
//        // VRAM
//        return m_vram->read_n8(address);
//    } else if (address < 0xC000) {
//        // External RAM
//        return m_wram->read_n8(address);
//    } else if (address < 0xE000) {
//        // WRAM
//        return m_wram->read_n8(address);
//    } else if (address < 0xFE00) {
//        // Echo RAM
//        return m_wram->read_n8(address);
//    } else if (address < 0xFF00) {
//        // OAM
//        return m_oam->read_n8(address);
//    } else if (address < 0xFF80) {
//        // IO Registers
//        return m_io->read_n8(address);
//    } else {
//        // HRAM
//        return m_wram->read_n8(address);
//    }
}

void Bus::write_n8(uint16_t address, uint8_t data) {
    if (address < 0x8000) {
        // ROM
        m_cartridge->write_n8(address, data);
    }
}

void Bus::write_n16(uint16_t address, uint16_t data) {
    if (address < 0x8000) {
        // ROM
        m_cartridge->write_n16(address, data);
    }
}
