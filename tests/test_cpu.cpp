#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <filesystem>
#include "cpu.h"
#include "rom.h"

class TestCPU {
public:
    ROM *rom = new ROM();
    Bus *bus = new Bus(rom);
    CPU *cpu = new CPU(bus);

    ~TestCPU() { delete cpu; }

    uint8_t *get_r8_from_opcode(uint8_t opcode, size_t start_pos = 0) {
        return cpu->get_r8_from_opcode(opcode, start_pos);
    }

    Register *get_r16_from_opcode(uint8_t opcode, size_t start_pos = 0) {
        return cpu->get_r16_from_opcode(opcode, start_pos);
    }

    template<typename T> void add(T a, T b, T &result) {
        cpu->add(a, b, result);
    }

    template<typename T> void sub(T a, T b, T &result) {
        cpu->sub(a, b, result);
    }
};

TEST_CASE("Register union test") {
    Register reg { .r16 = 0xABCD };
    REQUIRE(reg.r8.lo == 0xCD);
    REQUIRE(reg.r8.hi == 0xAB);
    REQUIRE(reg.r16 == 0xABCD);
}

TEST_CASE("CPUFlags union test") {
    CPUFlags flags { .flags = 0b00001111 };
    REQUIRE(flags.bits.c == 1);
    REQUIRE(flags.bits.h == 1);
    REQUIRE(flags.bits.n == 1);
    REQUIRE(flags.bits.z == 1);
    REQUIRE(flags.bits.unused == 0b0000);
    REQUIRE(flags.flags == 0b00001111);
}

TEST_CASE("ROM loading test") {
    ROM rom;
    std::fstream test_rom("roms/test_rom.gb", std::ios::in | std::ios::out | std::ios::binary);
    if (!test_rom) {
        test_rom.open("roms/test_rom.gb", std::ios::out | std::ios::binary);
        uint8_t test_data[0x8000] = { 0xFF };
        test_data[0x0001] = 0x00;
        test_rom.write(reinterpret_cast<char*>(test_data), sizeof(test_data));
        test_rom.close();
    }

    SECTION("Load valid ROM") {
        rom.load("roms/test_rom.gb");
        REQUIRE(rom.read_n8(0x0000) == 0xFF);
        REQUIRE(rom.read_n8(0x0001) == 0x00);
    }

    std::filesystem::remove("roms/test_rom.gb");
}

TEST_CASE("CPU get_r16_from_opcode test") {
    TestCPU test_cpu;
    Register *reg = test_cpu.get_r16_from_opcode(0xD5);
    REQUIRE(reg == &test_cpu.cpu->get_state().DE);
}

TEST_CASE("CPU get_r8_from_opcode test") {
    TestCPU test_cpu;
    uint8_t *reg = test_cpu.get_r8_from_opcode(0x54);
    REQUIRE(reg == &test_cpu.cpu->get_state().HL.r8.hi);

    uint8_t *reg2 = test_cpu.get_r8_from_opcode(0x55);
    REQUIRE(reg2 == &test_cpu.cpu->get_state().HL.r8.lo);
}

TEST_CASE("CPU add and sub test") {
    TestCPU test_cpu;
    uint8_t a = 0x05, b = 0x03, result = 0;

    SECTION("Addition test") {
        test_cpu.add(a, b, result);
        REQUIRE(result == 0x08);
        REQUIRE(test_cpu.cpu->get_state().FLAGS.bits.z == 0);
        REQUIRE(test_cpu.cpu->get_state().FLAGS.bits.n == 0);
        REQUIRE(test_cpu.cpu->get_state().FLAGS.bits.h == 0);
        REQUIRE(test_cpu.cpu->get_state().FLAGS.bits.c == 0);
    }

    SECTION("Subtraction test") {
        test_cpu.sub(a, b, result);
        REQUIRE(result == 0x02);
        REQUIRE(test_cpu.cpu->get_state().FLAGS.bits.z == 0);
        REQUIRE(test_cpu.cpu->get_state().FLAGS.bits.n == 1);
        REQUIRE(test_cpu.cpu->get_state().FLAGS.bits.h == 0);
        REQUIRE(test_cpu.cpu->get_state().FLAGS.bits.c == 0);
    }
}

TEST_CASE("CPU step test") {
    TestCPU test_cpu;
    test_cpu.cpu->reset();

    test_cpu.cpu->get_state().PC.r16 = 0x0000;
    test_cpu.bus->write_n8(0x0000, 0xC3);  // JP n16
    test_cpu.bus->write_n8(0x0001, 0x34);
    test_cpu.bus->write_n8(0x0002, 0x12);

    test_cpu.cpu->step();

    REQUIRE(test_cpu.cpu->get_state().PC.r16 == 0x1234);
}
