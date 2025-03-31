#pragma once

#include <iostream>
#include <cstdint>
#include <fstream>
#include <string>

class ROM {
public:
    ROM() {};
    ~ROM() = default;

    /*
     * load - Loads data into the ROM from a file.
     * @filename: the name of the file to load into the ROM.
     */
    void load(std::string filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Unable to open file " << filename << std::endl;
            return;
        }
        file.read(reinterpret_cast<char*>(m_data), sizeof(m_data));
    }

    /*
     * load - Loads data into the ROM from a byte array.
     * @data: the byte array to load into the ROM.
     * @size: the size of the data to load.
     */
    void load(uint8_t *data, size_t size) {
        if (size > sizeof(m_data)) {
            std::cerr << "Error: Data size exceeds ROM size." << std::endl;
            return;
        }
        std::copy(data, data + size, m_data);
    }

    /*
     * read_n8 - Reads an 8-bit value from the ROM at the specified address.
     * @address: the address to read from.
     *   
     *   Return: the 8-bit value read from the ROM.
     */
    uint8_t read_n8(uint16_t address) {
        if (address < 0x8000) {
            return m_data[address];
        } else {
            std::cerr << "Error: Attempt to read from ROM outside of valid range." << std::endl;
            return 0;
        }
    }

    /*
     * read_n16 - Reads a 16-bit value from the ROM at the specified address.
     * @address: the address to read from.
     *   
     *   Return: the 16-bit value read from the ROM.
     */
    uint16_t read_n16(uint16_t address) {
        if (address < 0x8000) {
            return (m_data[address] | (m_data[address + 1] << 8));
        } else {
            std::cerr << "Error: Attempt to read from ROM outside of valid range." << std::endl;
            return 0;
        }
    }

    /*
     * write_n8 - Writes an 8-bit value to the ROM at the specified address.
     * @address: the address to write to.
     * @data:    the 8-bit value to write.
     */
    void write_n8(uint16_t address, uint8_t data) {
        if (address < 0x8000) {
            m_data[address] = data;
        } else {
            std::cerr << "Error: Attempt to write to ROM outside of valid range." << std::endl;
        }
    }

    /*
     * write_n16 - Writes a 16-bit value to the ROM at the specified address.
     * @address: the address to write to.
     * @data:    the 16-bit value to write.
     */
    void write_n16(uint16_t address, uint16_t data) {
        if (address < 0x8000) {
            m_data[address] = data & 0xFF;
            m_data[address + 1] = (data >> 8) & 0xFF;
        } else {
            std::cerr << "Error: Attempt to write to ROM outside of valid range." << std::endl;
        }
    }

private:
    uint8_t m_data[0x8000] = {0}; // 32KB ROM
};

