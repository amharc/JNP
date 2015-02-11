#ifndef _CPU_H
#define _CPU_H

#include <memory>
#include "common.h"
#include "memory.h"
#include "process.h"

namespace computer_internal {
class CPU {
    public:
    using interrupt_handler_type = std::function<void()>;

    private:
    RegisterSetPtr registers;
    RAMPtr ram;

    time_type timer;
    bool timer_active;

    interrupt_handler_type interrupt_handler;
    ProcessPtr job;
    bool awake;

    enum class ProtectionLevel { RING0, RING3 } current_level;

    void requireLevel(ProtectionLevel level);
    void interrupt();
    void timerTick();

    class restorer {
        private:
            CPU *cpu;
        public:
        restorer(CPU *cpu);
        ~restorer();
    };

    public:
    CPU(register_type register_count, RAMPtr ram = RAMPtr{});
    CPU(const CPU&);
    CPU& operator=(const CPU&);
    void setRAM(RAMPtr ram);
    void clearRegisters();
    void setInterruptHandler(interrupt_handler_type handler);
    void sleep();
    void setJob(ProcessPtr process);
    void awaken();
    void setTimer(time_type left);
    void disableTimer();
};
} // namespace computer_internal

#endif // _CPU_H
