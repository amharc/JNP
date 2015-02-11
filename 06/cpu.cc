#include "cpu.h"

namespace computer_internal {
void CPU::requireLevel(ProtectionLevel level) {
    if(level < current_level)
        throw std::logic_error("General protection fault");
}

void CPU::interrupt() {
    current_level = ProtectionLevel::RING0;
    interrupt_handler();
    current_level = ProtectionLevel::RING3;
}

void CPU::timerTick() {
    if(timer_active && --timer == 0) {
        timer_active = false;
        interrupt();
    }
}

CPU::restorer::restorer(CPU *cpu) : cpu{cpu} { }

CPU::restorer::~restorer() {
    cpu->awake = false;
    cpu->current_level = ProtectionLevel::RING0;
    cpu->timer_active = false;
}

CPU::CPU(register_type register_count, RAMPtr ram)
    : registers{std::make_shared<RegisterSet>(register_count)}
    , ram{ram}
    , timer{0}
    , timer_active{false}
    , awake{false}
    , current_level{ProtectionLevel::RING0}
    { }

CPU::CPU(const CPU &that)
    : registers{std::make_shared<RegisterSet>(*that.registers)}
    , ram{}
    , timer{0}
    , timer_active{false}
    , awake{false}
    , current_level{ProtectionLevel::RING0}
    { }

CPU& CPU::operator=(const CPU &that) {
    registers = std::make_shared<RegisterSet>(*that.registers);
    ram = nullptr;
    timer = 0;
    timer_active = false;
    awake = false;
    current_level = ProtectionLevel::RING0;

    return *this;
}

void CPU::setRAM(RAMPtr ram) {
    this->ram = ram;
}

void CPU::clearRegisters() {
    registers->clear();
}

void CPU::setInterruptHandler(interrupt_handler_type handler) {
    requireLevel(ProtectionLevel::RING0);
    interrupt_handler = handler;
}

void CPU::sleep() {
    requireLevel(ProtectionLevel::RING0);
    awake = false;
}

void CPU::setJob(ProcessPtr process) {
    requireLevel(ProtectionLevel::RING0);
    job = process;
}

void CPU::awaken() {
    requireLevel(ProtectionLevel::RING0);

    awake = true;
    restorer graceful_exit{this};

    while(awake) {
        if(!job || !job->hasNext()) {
            interrupt();
            continue;
        }

        job->next()->execute(registers, ram);
        timerTick();
    }
}

void CPU::setTimer(time_type left) {
    requireLevel(ProtectionLevel::RING0);
    timer_active = true;
    timer = left;
}

void CPU::disableTimer() {
    requireLevel(ProtectionLevel::RING0);
    timer_active = false;
}
} // namespace computer_internal
