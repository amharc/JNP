#ifndef _QUATERNION_SEQUENCE_H
#define _QUATERNION_SEQUENCE_H

#include <atomic>
#include <functional>
#include <iostream>
#include <map>
#include <vector>
#include "quaternion.h"

class QuaternionSequence {
    public:
    using size_type = size_t;
    using count_type = size_t;

    QuaternionSequence();

    // The following three constructors are explicit, because accidental conversion
    // of a map or a vector to a QuaternionSequence is dangerous
    explicit QuaternionSequence(const std::map<size_type, Quaternion>&);
    explicit QuaternionSequence(std::map<size_type, Quaternion>&&);
    explicit QuaternionSequence(const std::vector<Quaternion>&);

    QuaternionSequence(const QuaternionSequence&);
    QuaternionSequence(QuaternionSequence&&);

    virtual ~QuaternionSequence();

    QuaternionSequence& operator=(const QuaternionSequence&) = default;
    QuaternionSequence& operator=(QuaternionSequence&&) = default;

    QuaternionSequence& operator+=(const QuaternionSequence&);
    QuaternionSequence& operator-=(const QuaternionSequence&);
    QuaternionSequence& operator*=(const QuaternionSequence&);
    QuaternionSequence& operator*=(const Quaternion&) noexcept;

    const Quaternion& operator[](size_type) const;

    void insert(size_type, const Quaternion&);
    void insert(size_type, Quaternion&&);

    bool operator==(const QuaternionSequence&) const;
    bool operator!=(const QuaternionSequence&) const;

    explicit operator bool() const noexcept;

    static count_type count() noexcept;

    friend std::ostream& operator<<(std::ostream&, const QuaternionSequence&);
    friend const QuaternionSequence operator*(const QuaternionSequence&,
            const Quaternion&);
    friend const QuaternionSequence operator*(const Quaternion&,
            const QuaternionSequence&);

    private:
    // Number of existing QuaternionSequences. This value is atomic to ensure
    // thread-safety of some of the QuaternionSequence operations
    static std::atomic<count_type> active;
    // Auxiliary zero quaternion
    static const Quaternion zero;
    // Hold non-zero elements of the sequence, indexed by their position
    std::map<size_type, Quaternion> map;

    // Auxiliary function used in operator+ and operator-
    // merges two QuaternionSequences using a function f, provided that f(x, 0) = x
    void binop(const QuaternionSequence&,
            const std::function<Quaternion(const Quaternion&, const Quaternion&)> &f);
};

// First arguments are passed by value
const QuaternionSequence operator+(QuaternionSequence, const QuaternionSequence&);
const QuaternionSequence operator-(QuaternionSequence, const QuaternionSequence&);
const QuaternionSequence operator*(QuaternionSequence, const QuaternionSequence&);


#endif // _QUATERNION_SEQUENCE_H
