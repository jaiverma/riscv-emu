#include <iostream>
#include <array>
#include <vector>
#include <fstream>
#include <iomanip>
#include <stdint.h>

// 128 MB
#define MEMORY_SIZE 1024 * 1024 * 128

struct Cpu {
    // data
    std::array<uint64_t, 32> regs;
    uint64_t pc;
    std::vector<uint8_t> memory;

    // methods
    Cpu(std::vector<uint8_t>& binary);
    void dump_registers();
    uint32_t fetch();
    void execute(uint32_t inst);
};

Cpu::Cpu(std::vector<uint8_t>& binary) {
    regs.fill(0);
    regs[2] = MEMORY_SIZE;

    memory = binary;
    pc = 0;
}

void Cpu::dump_registers() {
    std::string output;

    for (int i = 0; i < regs.size(); i+= 4) {
        std::cout << "x" << std::dec << i << "=" << std::hex << regs[i]
            << " x" <<  std::dec << i + 1 << "=" << std::hex << regs[i+1]
            << " x" <<  std::dec << i + 2 << "=" << std::hex << regs[i+2]
            << " x" <<  std::dec << i + 3 << "=" << std::hex << regs[i+3]
            << std::endl;
    }
}

uint32_t Cpu::fetch() {
    return memory[pc] |
        memory[pc + 1] << 8  |
        memory[pc + 2] << 16 |
        memory[pc + 3] << 24;
}

void Cpu::execute(uint32_t inst) {
    uint32_t opcode = inst & 0x0000007f;
    uint32_t rd = (inst & 0x00000f80) >> 7;             // 0b11111 << 7
    uint32_t rs1 = (inst & 0xf8000) >> 15;              // 0b11111 << 15
    uint32_t rs2 = (inst & 0x1f00000) >> 20;            // 0b11111 << 20

    regs[0] = 0;

    switch (opcode) {
        case 0x13: {
            // addi
            int32_t imm = (inst & 0xfff00000) >> 20;   // 0b111111111111 << 20
            regs[rd] = regs[rs1] + imm;
            break;
        }

        case 0x33:
            // add
            regs[rd] = regs[rs1] + regs[rs2];
            break;

        default:
            std::cerr << "not implemented: " << std::hex << opcode
                << std::endl;
            break;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <binary>\n";
        return 1;
    }

    std::ifstream stream(argv[1], std::ios::in | std::ios::binary);
    std::vector<uint8_t> binary((std::istreambuf_iterator<char>(stream)),
        std::istreambuf_iterator<char>());

    std::cout << "read " << binary.size() << " bytes\n";

    Cpu cpu(binary);

    while (cpu.pc < cpu.memory.size()) {
        uint32_t inst = cpu.fetch();

        cpu.pc += 4;

        cpu.execute(inst);
    }

    cpu.dump_registers();

    return 0;
}
