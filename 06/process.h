#ifndef _PROCESS_H
#define _PROCESS_H

#include <list>
#include <memory>
#include "common.h"
#include "instruction.h"

namespace computer_internal {
using Program = std::list<std::shared_ptr<const Instruction>>;
using ProgramPtr = std::shared_ptr<Program>;

class Process {
    public:
    private:
    ProgramPtr text;
    Program::const_iterator instruction_pointer;

    public:
    explicit Process(const ProgramPtr &text);
    const ProgramPtr& program();

    bool hasNext();
    // Picks the next instruction and increments the instruction pointer
    std::shared_ptr<const Instruction> next();
};

using ProcessPtr = std::shared_ptr<Process>;
} // namespace computer_internal

#endif // _PROCESS_H
