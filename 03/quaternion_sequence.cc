#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <map>
#include <vector>
#include <utility>
#include "quaternion.h"
#include "quaternion_sequence.h"

std::atomic<QuaternionSequence::count_type> QuaternionSequence::active;
const Quaternion QuaternionSequence::zero;

QuaternionSequence::QuaternionSequence() {
    ++active;
}

QuaternionSequence::QuaternionSequence(const std::map<size_type, Quaternion> &map) {
    ++active;

    for(const auto &pair : map) {
        if(pair.second)
            this->map.insert(this->map.cend(), pair);
    }
}

QuaternionSequence::QuaternionSequence(std::map<size_type, Quaternion> &&map)
    : map(std::move(map)) {
    ++active;

    // Remove zeroes
    auto it = this->map.begin();
    while(it != this->map.end()) {
        if(!it->second)
            it = this->map.erase(it);
        else
            ++it;
    }
}

QuaternionSequence::QuaternionSequence(const std::vector<Quaternion> &vector) {
    ++active;

    for(std::vector<Quaternion>::size_type i = 0; i < vector.size(); ++i)
        if(vector[i])
            map.emplace(i, vector[i]);
}

QuaternionSequence::QuaternionSequence(const QuaternionSequence &seq)
    : map(seq.map) {
    ++active;
}

QuaternionSequence::QuaternionSequence(QuaternionSequence &&seq)
    : map(std::move(seq.map)) {
    ++active;
}

QuaternionSequence::~QuaternionSequence() {
    --active;
}

QuaternionSequence& QuaternionSequence::operator+=(const QuaternionSequence &seq) {
    binop(seq, std::plus<Quaternion>{});
    return *this;
}

QuaternionSequence& QuaternionSequence::operator-=(const QuaternionSequence &seq) {
    binop(seq, std::minus<Quaternion>{});
    return *this;
}

// The multiplication functions use the fact that the ring of quaternions is a
// domain, i.e. it has no nontrivial zero divisors

// The following function does not use the function binop, because multiplication,
// unlike addition and subtraction does not respect the identity x * 0 = x,
// on which binop relies. Instead it traverses this->map, using the identity
// 0 * x = 0
QuaternionSequence& QuaternionSequence::operator*=(const QuaternionSequence &seq) {
    auto it = map.begin();

    while(it != map.end()) {
        auto seq_it = seq.map.find(it->first);
        if(seq_it == seq.map.end()) // multiplication by zero
            it = map.erase(it);
        else {
            it->second *= seq_it->second;
            ++it;
        }
    }

    return *this;
}

QuaternionSequence& QuaternionSequence::operator*=(const Quaternion &q) noexcept {
    if(!q)
        map.clear();
    else {
        for(auto &pair : map) {
            pair.second *= q;
        }
    }

    return *this;
}

const Quaternion& QuaternionSequence::operator[](size_type index) const {
    auto it = map.find(index);
    if(it == map.end())
        // Here zero is returned, because returning reference to temporary is not
        // possible
        return zero;
    else
        return it->second;
}

void QuaternionSequence::insert(size_type index, const Quaternion &q) {
    if(q)
        map[index] = q;
    else
        map.erase(index);
}

void QuaternionSequence::insert(size_type index, Quaternion &&q) {
    if(q)
        map[index] = std::move(q);
    else
        map.erase(index);
}

bool QuaternionSequence::operator==(const QuaternionSequence &seq) const {
    return map == seq.map;
}

bool QuaternionSequence::operator!=(const QuaternionSequence &seq) const {
    return map != seq.map;
}

QuaternionSequence::operator bool() const noexcept {
    return !map.empty();
}

QuaternionSequence::count_type QuaternionSequence::count() noexcept {
    return active;
}

std::ostream& operator<<(std::ostream &os, const QuaternionSequence &seq) {
    os << "(";
    if(!seq.map.empty()) {
        auto max_index = seq.map.crbegin()->first;
        for(const auto &pair : seq.map) {
            os << pair.first << " -> " << pair.second;
            if(pair.first != max_index)
                os << ", ";
        }
    }
    os << ")";
    return os;
}

void QuaternionSequence::binop(const QuaternionSequence &seq,
        const std::function<Quaternion(const Quaternion&, const Quaternion&)> &fun) {
    // Because of the identity fun(x, 0) = x, we have to traverse only the second map
    for(const auto &pair : seq.map) {
        size_type index = pair.first;
        const Quaternion &q = pair.second;

        auto it = map.find(index);
        if(it == map.end())
            map.emplace(index, fun(zero, q));
        else {
            Quaternion res = fun(it->second, q);
            if(res)
                it->second = std::move(res);
            else
                map.erase(it);
        }
    }
}

const QuaternionSequence operator*(const QuaternionSequence &seq,
                                   const Quaternion &q) {
    if(!q)
        return {};

    decltype(seq.map) newmap;
    for(const auto &pair : seq.map) {
        newmap.emplace_hint(newmap.cend(), pair.first, pair.second * q);
    }

    return QuaternionSequence{std::move(newmap)};
}

const QuaternionSequence operator*(const Quaternion &q,
                                   const QuaternionSequence &seq) {
    if(!q)
        return {};

    decltype(seq.map) newmap;
    for(const auto &pair : seq.map) {
        newmap.emplace_hint(newmap.cend(), pair.first, q * pair.second);
    }

    return QuaternionSequence{std::move(newmap)};
}

// First arguments are passed by value
const QuaternionSequence operator+(QuaternionSequence lhs,
                                   const QuaternionSequence &rhs) {
    return lhs += rhs;
}

const QuaternionSequence operator-(QuaternionSequence lhs,
                                   const QuaternionSequence &rhs) {
    return lhs -= rhs;
}

const QuaternionSequence operator*(QuaternionSequence lhs,
                                   const QuaternionSequence &rhs) {
    return lhs *= rhs;
}
