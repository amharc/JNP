#include "process.h"

namespace computer_internal {
Process::Process(const ProgramPtr &text)
    : text{text}, instruction_pointer{this->text->cbegin()} { }

bool Process::hasNext() {
    return instruction_pointer != text->cend();
}

const ProgramPtr& Process::program() {
    return text;
}

std::shared_ptr<const Instruction> Process::next() {
    return *(instruction_pointer++);
}
} // namespace computer_internal
