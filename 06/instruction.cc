#include "instruction.h"

#include <iostream>

namespace computer_internal {
Instruction::~Instruction() { }

SetInstruction::SetInstruction(register_type reg, number_type val)
    : reg{reg}, val{val} { }

void SetInstruction::execute(RegisterSetPtr regs, RAMPtr) const {
    regs->store(reg, val);
}

LoadInstruction::LoadInstruction(register_type dest, memory_type src)
    : dest{dest}, src{src} { }

void LoadInstruction::execute(RegisterSetPtr regs, RAMPtr ram) const {
    number_type val = ram->load(src);
    regs->store(dest, val);
}

StoreInstruction::StoreInstruction(memory_type dest, register_type src)
    : dest{dest}, src{src} { }

void StoreInstruction::execute(RegisterSetPtr regs, RAMPtr ram) const {
    number_type val = regs->load(src);
    ram->store(dest, val);
}

PrintlnInstruction::PrintlnInstruction(register_type reg) : reg{reg} { }

void PrintlnInstruction::execute(RegisterSetPtr regs, RAMPtr) const {
    number_type val = regs->load(reg);
    std::cout << val << std::endl;
}
} // namespace computer_internal
