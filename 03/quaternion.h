#ifndef _QUATERNION_H
#define _QUATERNION_H

#include <cmath>
#include <iostream>

class Quaternion {
    public:
    constexpr Quaternion() noexcept : Quaternion(0, 0, 0, 0) { }
    // The following constructor is not explicit, because we want to have implicit
    // conversion from double to Quaternion
    constexpr Quaternion(double r) noexcept : Quaternion(r, 0, 0, 0) { }
    constexpr Quaternion(double re, double im) noexcept : Quaternion(re, im, 0, 0) { }
    constexpr Quaternion(double r, double i, double j, double k) noexcept
        : r(r), i(i), j(j), k(k) { }

    // Although C++11 marks every constexpr method as const, C++14 does not,
    // so the relevant const specifier is stated explicitly
    constexpr double R() const noexcept { return r; }
    constexpr double I() const noexcept { return i; }
    constexpr double J() const noexcept { return j; }
    constexpr double K() const noexcept { return k; }

    constexpr const Quaternion conj() const noexcept {
        return Quaternion(r, -i, -j, -k);
    }

    // This function is not constexpr, because std::sqrt is not
    double norm() const noexcept {
        return std::sqrt(r * r + i * i + j * j + k * k);
    }

    constexpr const Quaternion operator-() const noexcept {
        return Quaternion(-r, -i, -j, -k);
    }

    constexpr const Quaternion operator+() const noexcept {
        return *this;
    }

    Quaternion& operator+=(const Quaternion &q) noexcept {
        r += q.r;
        i += q.i;
        j += q.j;
        k += q.k;
        return *this;
    }

    Quaternion& operator-=(const Quaternion &q) noexcept {
        r -= q.r;
        i -= q.i;
        j -= q.j;
        k -= q.k;
        return *this;
    }

    Quaternion& operator*=(const Quaternion &q) noexcept {
        double nr = r * q.r - i * q.i - j * q.j - k * q.k;
        double ni = r * q.i + i * q.r + j * q.k - k * q.j;
        double nj = r * q.j - i * q.k + j * q.r + k * q.i;
        double nk = r * q.k + i * q.j - j * q.i + k * q.r;
        r = nr, i = ni, j = nj, k = nk;
        return *this;
    }

    constexpr explicit operator bool() const noexcept {
        return r != 0 || i != 0 || j != 0 || k != 0;
    }

    private:
    double r, i, j, k;
};

// constexpr objects are const, even in C++14, so they have internal linkage
constexpr Quaternion I(0, 1, 0, 0);
constexpr Quaternion J(0, 0, 1, 0);
constexpr Quaternion K(0, 0, 0, 1);

// The following functions are inline, because then they are treated specially
// in the One Definition Rule, namely then can be defined more than once,
// albeit in different translation units
constexpr inline bool operator==(const Quaternion &lhs, const Quaternion &rhs) noexcept {
    return lhs.R() == rhs.R() &&
           lhs.I() == rhs.I() &&
           lhs.J() == rhs.J() &&
           lhs.K() == rhs.K();
}

constexpr inline bool operator!=(const Quaternion &lhs, const Quaternion &rhs) noexcept {
    return !(lhs == rhs);
}

// This function is not constexpr, because Quaternion::norm is not
inline double norm(const Quaternion &q) noexcept {
    return q.norm();
}

constexpr inline const Quaternion conj(const Quaternion &q) noexcept {
    return q.conj();
}

// First arguments are passed by value in the following three operators
inline const Quaternion operator+(Quaternion lhs, const Quaternion &rhs) noexcept {
    return lhs += rhs;
}

inline const Quaternion operator-(Quaternion lhs, const Quaternion &rhs) noexcept {
    return lhs -= rhs;
}

inline const Quaternion operator*(Quaternion lhs, const Quaternion &rhs) noexcept {
    return lhs *= rhs;
}

inline std::ostream& operator<<(std::ostream &os, const Quaternion &q) {
    // Indicates whether something was already written to the stream
    bool first = true;

    // An auxiliary function handling the writing of one coordinate.
    // A nonzero coordinate is writtern in three steps: sign, magnitude, unit
    // d -- the coefficient
    // l -- the c-string to be written next to the coefficient, i.e. the unit
    auto print = [&](double d, const char *l) {
        if(d != 0) {
            if(d > 0 && !first)
                os << '+';
            else if(d < 0)
                os << '-';

            if(std::abs(d) != 1 || l[0] == 0)
                os << std::abs(d);

            os << l;
            first = false;
        }
    };

    print(q.R(), "");
    print(q.I(), "i");
    print(q.J(), "j");
    print(q.K(), "k");

    if(first) // Nothing has been written yet, so q == 0
        os << "0";

    return os;
}

#endif // _QUATERNION_H
