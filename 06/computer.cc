#include "computer.h"

using namespace computer_internal;

Computer::Computer() : changes_disabled{false} { }
Computer::Computer(const Computer &that) : changes_disabled{false} {
    *this = that;
}

Computer& Computer::operator=(const Computer &that) {
    if(changes_disabled)
        throw IllegalChangeException();

    decltype(cpu) newcpu = nullptr;
    decltype(ram) newram = nullptr;

    if(that.cpu)
        newcpu = std::make_shared<CPU>(*that.cpu);

    if(that.ram) {
        newram = std::make_shared<RAM>(*that.ram);
        if(newcpu)
            newcpu->setRAM(newram);
    }

    // Now entering the non-throwing part
    cpu = newcpu;
    ram = newram;
    return *this;
}

void Computer::setCPU(register_type numOfRegisters) {
    if(changes_disabled)
        throw IllegalChangeException();

    cpu = std::make_shared<CPU>(numOfRegisters, ram);
}

void Computer::setRAM(memory_type size) {
    if(changes_disabled)
        throw IllegalChangeException();

    ram = std::make_shared<RAM>(size);
    if(cpu)
        cpu->setRAM(ram);
}

std::shared_ptr<OS> Computer::installOS(std::shared_ptr<SchedulingAlgorithm> alg) {
    if(!ram)
        throw NoRAMException();
    if(!cpu)
        throw NoCPUException();

    cpu->clearRegisters();
    ram->clear();
    changes_disabled = true;

    return std::shared_ptr<OS>{new OS{cpu, alg}};
}
