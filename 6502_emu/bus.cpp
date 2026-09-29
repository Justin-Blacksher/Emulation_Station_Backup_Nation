#include "bus.h"

Bus::Bus() {
    // Clear out RAM upon initialization
    ram.fill(0);
}

u8t Bus::read(u16t addr, bool readOnly) {
    // In a complete system, you might check if addr falls in RAM or PPU ranges here. TODO: Have AI help
    if (addr < ram.size()) {
        return ram[addr];
    }
    return 0x00;
}

void Bus::write(u16t addr, u8t data) {
    if (addr < ram.size()) {
        ram[addr] = data;
    }
}

void Bus::load_program(const std::initializer_list<u8t>& program, u16t start_addr) {
    u16t addr = start_addr;
    for (u8t byte : program) {
        write(addr++, byte);
    }
}

