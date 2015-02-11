#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <list>
#include <memory>
#include <utility>
#include "common.h"
#include "process.h"

namespace computer_internal {
// Forward declaration
class Scheduler;
}

class SchedulingAlgorithm {
    private:
    // Needed for operator= and copy constructor to work correctly
    // (SchedulingAlgorithm is only a wrapper around Scheduler)
    std::shared_ptr<computer_internal::Scheduler> implementation;

    public:
    constexpr static const time_type WITHOUT_TIMER = 0;

    // <which process should run next, the quantum allocated>
    // null shared pointer is passed as the process pointer when
    // the CPU should be halted. Quantum may be equal to WITHOUT_TIMER
    using response_type = std::pair<computer_internal::ProcessPtr, time_type>;
    using list_type = std::list<computer_internal::ProcessPtr>;

    SchedulingAlgorithm(std::shared_ptr<computer_internal::Scheduler> implementation);
    void setList(std::unique_ptr<list_type> processes) const;
    response_type schedule() const;
};


namespace computer_internal {
// The real scheduler
class Scheduler {
    public:
    using list_type = SchedulingAlgorithm::list_type;
    using response_type = SchedulingAlgorithm::response_type;

    protected:
    std::unique_ptr<list_type> active;
    list_type::iterator current;

    // Deriving classes may want to perform some action when the list is
    // changed (e.g. sort it according to some fancy predicate)
    virtual void listChanged();

    // The function which picks the process (deriving classes SHOULD pick
    // *current, but they are not required to (as in RFC2119)) and decides
    // how long should it run.
    virtual response_type pickProcess() = 0;

    public:
    void setList(std::unique_ptr<list_type> processes);
    response_type schedule();
    virtual ~Scheduler();
};

class FCFSScheduler : public Scheduler {
    public:
    virtual response_type pickProcess() override;
};

class RRScheduler : public Scheduler {
    private:
    time_type quantum;

    public:
    RRScheduler(time_type quantum);
    virtual response_type pickProcess() override;
};

class SJFScheduler : public Scheduler {
    private:
    // Auxiliary functions used to sort the list of processes
    static auto length(const ProcessPtr &p) -> decltype(p->program()->size());
    static bool compare(const ProcessPtr &lhs, const ProcessPtr &rhs);

    protected:
    virtual void listChanged() override;
    
    public:
    virtual response_type pickProcess() override;
};
} // namespace computer_internal

std::shared_ptr<SchedulingAlgorithm> createFCFSScheduling();
std::shared_ptr<SchedulingAlgorithm> createRRScheduling(time_type quantum);
std::shared_ptr<SchedulingAlgorithm> createSJFScheduling();
#endif // _SCHEDULER_H
