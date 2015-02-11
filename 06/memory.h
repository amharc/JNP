#ifndef _MEMORY_H
#define _MEMORY_H

#include <algorithm>
#include <memory>
#include <vector>
#include "common.h"

namespace computer_internal {
template<unsigned From, class Index, class Value, class Exception>
class Memory {
    private:
    std::vector<Value> mem;

    Value& get(Index idx) {
        Index aligned = idx - From;
        if(aligned < 0 || aligned >= static_cast<Index>(mem.size()))
            throw Exception(idx);

        return mem[aligned];
    }

    public:
    Memory(Index size) {
        if(size <= 0)
            throw IllegalArgumentException("Negative size provided");
        mem.resize(size);
    }

    void store(Index idx, Value val) {
        get(idx) = val;
    }

    Value load(Index idx) {
        return get(idx);
    }

    void clear() {
        std::fill(mem.begin(), mem.end(), 0);
    }
};

using RegisterSet = Memory<1, register_type, number_type, InvalidRegisterException>;
using RAM = Memory<0, memory_type, number_type, InvalidAddressException>;

using RegisterSetPtr = std::shared_ptr<RegisterSet>;
using RAMPtr = std::shared_ptr<RAM>;
} // namespace computer_internal

#endif // _MEMORY_H
