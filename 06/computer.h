#ifndef _COMPUTER_H
#define _COMPUTER_H

#include <memory>
#include "common.h"
#include "cpu.h"
#include "memory.h"
#include "scheduler.h"
#include "os.h"
#include "forward.h"

class Computer {
    private:
    // If true, every change (such as setting CPU or RAM) will result in an
    // exception
    bool changes_disabled;
    std::shared_ptr<computer_internal::CPU> cpu;
    std::shared_ptr<computer_internal::RAM> ram;

    public:
    Computer();

    // Copy {constructor,assignment operator} perform a deep copy, i.e. the
    // RAM and CPU are copied themselves, not merely the pointers to them
    Computer(const Computer&);
    Computer& operator=(const Computer&);

    void setCPU(register_type numOfRegisters);
    void setRAM(memory_type size);
    std::shared_ptr<OS> installOS(std::shared_ptr<SchedulingAlgorithm> alg);
};

#endif
