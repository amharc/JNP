#include "scheduler.h"

namespace computer_internal {
void Scheduler::listChanged() { }

void Scheduler::setList(std::unique_ptr<list_type> processes) {
    active = std::move(processes);
    listChanged();

    // Current should point to the last process, it will be incremented
    // when schedule will be called
    current = active->end();
    if(current != active->begin())
        --current;
}

std::pair<ProcessPtr, time_type> Scheduler::schedule() {
    // Indicates whether a process was deleted from the active processes list
    // (and therefore the current-process-iterator should not be incremented)
    bool deleted = false;

    while(current != active->end() && !(*current)->hasNext()) {
        current = active->erase(current);
        deleted = true;
    }

    if(!deleted && current != active->end())
        ++current;

    if(current == active->end())
        current = active->begin();

    if(active->empty()) // finished! :)
        return {{}, SchedulingAlgorithm::WITHOUT_TIMER};
    else
        return pickProcess();
}

Scheduler::~Scheduler() { }

std::pair<ProcessPtr, time_type> FCFSScheduler::pickProcess() {
    return {*current, SchedulingAlgorithm::WITHOUT_TIMER};
}

RRScheduler::RRScheduler(time_type quantum) : quantum{quantum} { }

std::pair<ProcessPtr, time_type> RRScheduler::pickProcess() {
    return {*current, quantum};
}

auto SJFScheduler::length(const ProcessPtr &p) -> decltype(p->program()->size()) {
    return p->program()->size();
}

bool SJFScheduler::compare(const ProcessPtr &lhs, const ProcessPtr &rhs) {
    return length(lhs) < length(rhs);
}

void SJFScheduler::listChanged() {
    active->sort(compare);
}

std::pair<ProcessPtr, time_type> SJFScheduler::pickProcess() {
    return {*current, SchedulingAlgorithm::WITHOUT_TIMER};
}

} // namespace computer_internal

using namespace computer_internal;
constexpr const time_type SchedulingAlgorithm::WITHOUT_TIMER;

SchedulingAlgorithm::SchedulingAlgorithm(std::shared_ptr<Scheduler> implementation)
    : implementation{implementation} { }

void SchedulingAlgorithm::setList(std::unique_ptr<list_type> processes) const {
    implementation->setList(std::move(processes));
}

std::pair<ProcessPtr, time_type> SchedulingAlgorithm::schedule() const {
    return implementation->schedule();
}

std::shared_ptr<SchedulingAlgorithm> createFCFSScheduling() {
    auto scheduler = std::make_shared<FCFSScheduler>();
    return std::make_shared<SchedulingAlgorithm>(scheduler);
}

std::shared_ptr<SchedulingAlgorithm> createRRScheduling(time_type quantum) {
    auto scheduler = std::make_shared<RRScheduler>(quantum);
    return std::make_shared<SchedulingAlgorithm>(scheduler);
}

std::shared_ptr<SchedulingAlgorithm> createSJFScheduling() {
    auto scheduler = std::make_shared<SJFScheduler>();
    return std::make_shared<SchedulingAlgorithm>(scheduler);
}
