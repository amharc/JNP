#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include <functional>
#include "common.h"
#include "memory.h"

namespace computer_internal {
class Instruction {
    public:
    virtual void execute(RegisterSetPtr, RAMPtr) const = 0;
    virtual ~Instruction();
};

class SetInstruction : public Instruction {
    private:
    register_type reg;
    number_type val;

    public:
    SetInstruction(register_type reg, number_type val);
    virtual void execute(RegisterSetPtr regs, RAMPtr) const override;
};

class LoadInstruction : public Instruction {
    private:
    register_type dest;
    memory_type src;

    public:
    LoadInstruction(register_type dest, memory_type src);
    virtual void execute(RegisterSetPtr regs, RAMPtr ram) const override;
};

class StoreInstruction : public Instruction {
    private:
    memory_type dest;
    register_type src;

    public:
    StoreInstruction(memory_type dest, register_type src);
    virtual void execute(RegisterSetPtr regs, RAMPtr ram) const override;
};

template<class Op>
class ArithmeticInstruction : public Instruction {
    private:
    register_type dest;
    register_type src;
    Op operation;

    public:
    ArithmeticInstruction(register_type dest, register_type src)
        : dest{dest}, src{src} { }
    virtual void execute(RegisterSetPtr regs, RAMPtr) const override {
        number_type rhs = regs->load(src);
        number_type lhs = regs->load(dest);
        number_type res = operation(lhs, rhs);
        regs->store(dest, res);
    }
};

// long_number_type is used to avoid undefined behaviour caused by the
// signed integer overflow
using AddInstruction = ArithmeticInstruction<std::plus<long_number_type>>;
using SubInstruction = ArithmeticInstruction<std::minus<long_number_type>>;
using MulInstruction = ArithmeticInstruction<std::multiplies<long_number_type>>;

template<class T>
struct divides {
    T operator()(const T lhs, const T rhs) const {
        if(!rhs)
            throw DivisionByZeroException();
        return lhs / rhs;
    }
};

using DivInstruction = ArithmeticInstruction<divides<long_number_type>>;

class PrintlnInstruction : public Instruction {
    private:
    register_type reg;

    public:
    PrintlnInstruction(register_type reg);
    virtual void execute(RegisterSetPtr regs, RAMPtr) const override;
};
} // namespace computer_internal

#endif // _INSTRUCTION_H
