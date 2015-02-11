#ifndef _OS_H
#define _OS_H

#include <memory>
#include "cpu.h"
#include "common.h"
#include "scheduler.h"
#include "forward.h"

class OS {
    private:
    std::shared_ptr<computer_internal::CPU> cpu;
    std::shared_ptr<SchedulingAlgorithm> scheduler;

    static computer_internal::ProcessPtr makeProcess(const std::string &code);

    OS(std::shared_ptr<computer_internal::CPU> cpu,
       std::shared_ptr<SchedulingAlgorithm> scheduler);

    public:
    void executePrograms(const std::list<std::string> &programs);
    friend class Computer;
};

#endif // _OS_H
