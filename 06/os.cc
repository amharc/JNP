#include "os.h"
#include <algorithm>
#include <iterator>
#include "assembler.h"
#include "cpu.h"
#include "process.h"

using namespace computer_internal;

ProcessPtr OS::makeProcess(const std::string &code) {
    return std::make_shared<Process>(Assembler::compile(code));
}

OS::OS(std::shared_ptr<CPU> cpu, std::shared_ptr<SchedulingAlgorithm> scheduler)
    : cpu{cpu}, scheduler{scheduler} { }

void OS::executePrograms(const std::list<std::string> &programs) {
    using list_type = SchedulingAlgorithm::list_type;

    std::unique_ptr<list_type> list{new list_type{}};
    std::transform(programs.begin(),
                   programs.end(),
                   std::back_inserter<list_type>(*list),
                   makeProcess);
    scheduler->setList(std::move(list));

    // The interrupt handler
    auto schedule = [this]() {
        auto ret = scheduler->schedule();
        ProcessPtr process = ret.first;
        if(!process)
            cpu->sleep();
        else {
            time_type quantum = ret.second;

            if(quantum != SchedulingAlgorithm::WITHOUT_TIMER)
                cpu->setTimer(quantum);
            else
                cpu->disableTimer();

            cpu->setJob(process);
        }
    };

    cpu->disableTimer();
    cpu->setInterruptHandler(schedule);
    schedule();
    cpu->awaken();
    // Finished
    cpu->setInterruptHandler({});
}

