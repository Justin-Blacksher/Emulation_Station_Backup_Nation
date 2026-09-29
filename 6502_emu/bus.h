#pragma once
#include <array>
#include <cstdint>

typedef std::uint8_t u8t;
typedef std::uint16_t u16t;

class Bus {
    public:
    Bus();
    ~Bus() = default;

    // Core Bus Read/Write methods
    u8t read(u16t addr, bool readonly = false);
    void write(u16t addr, u8t data);

    // Convenience method to load a program into memory
    void load_program(const std::initializer_list<u8t>& program, u16t start_addr);

    // 64 KB of RAM
    std::array<u8t, 64 * 1024> ram;

};