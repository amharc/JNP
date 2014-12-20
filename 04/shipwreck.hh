#ifndef _SHIPWRECK_HH
#define _SHIPWRECK_HH

#include <algorithm>
#include <iostream>
#include <limits>
#include <tuple>

// NOTE: We assume that all appearing values fit into both int and unsigned int,
//       so integer overflows are not checked for

// Auxiliary templates and functions
namespace ship_utils {
    // std::min, std::max are not constexpr, so we have to write their constexpr
    // equivalents. Every T used in this code will be a primitive, so we may 
    // safely (and effectively) pass it by value.
    // The One Definition Rule does not apply to non-static function templates
    template<class T>
    constexpr T min(T q1, T q2) noexcept {
        return q1 > q2 ? q2 : q1;
    }

    template<class T, class... Rest>
    constexpr T min(T q, Rest... qs) noexcept {
        return min(q, min(qs...));
    }

    // Returns max(0, x). Constexpr implies inline
    constexpr unsigned max0(int x) noexcept {
        return x > 0 ? x : 0;
    }

    constexpr unsigned safe_sub(unsigned x, unsigned y) noexcept {
        return x > y ? x - y : 0;
    }
    
    // Division by zero returns infinity, i.e. the largest representable value
    constexpr unsigned safe_div(unsigned x, unsigned y) noexcept {
        return y == 0 ? std::numeric_limits<unsigned>::max() : x / y;
    }

    // Returns the weighted average of a and b with the respective weights
    // If both weights are zero, the result is the largest value representable
    // in the unsigned int type
    constexpr unsigned safe_avg(unsigned weight_a, unsigned a,
                                unsigned weight_b, unsigned b) noexcept {
        return safe_div(a * weight_a + b * weight_b, weight_a + weight_b);
    }

    // Converts a Gear to a tuple
    template<class Gear>
    constexpr std::tuple<unsigned, unsigned, unsigned> to_tuple() noexcept {
        // As the unary operator+ returns the value of its operand, it performs
        // an immediate lvalue-to-rvalue conversion of it, so e.g. Gear::cannons
        // is not being odr-used here (make_tuple accepts references, so simply
        // passing static const data member would constitute an illegal odr-use)
        return std::make_tuple(+Gear::cannons, +Gear::masts, +Gear::oars);
    }
}

template<unsigned Cannons, unsigned Masts, unsigned Oars>
struct ShipGear {
    // Note: those static data members have no namespace scope definitions,
    // so they must not be odr-used, but compiler is allowed not to allocate
    // memory for them
    constexpr static unsigned cannons = Cannons;
    constexpr static unsigned masts   = Masts;
    constexpr static unsigned oars    = Oars;
};

using Cannon = ShipGear<1, 0, 0>;
using Mast   = ShipGear<0, 1, 0>;
using Oar    = ShipGear<0, 0, 1>;

template<class Gear, class OtherGear>
struct add_gear {
    using type = ShipGear<Gear::cannons + OtherGear::cannons,
                          Gear::masts   + OtherGear::masts,
                          Gear::oars    + OtherGear::oars>;
};

template<class Gear, class OtherGear>
struct remove_gear {
    using type = ShipGear<ship_utils::safe_sub(Gear::cannons, OtherGear::cannons),
                          ship_utils::safe_sub(Gear::masts,   OtherGear::masts),
                          ship_utils::safe_sub(Gear::oars,    OtherGear::oars)>;
};

template<class Gear, unsigned N>
struct multiply_gear {
    using type = ShipGear<Gear::cannons * N, Gear::masts * N, Gear::oars * N>;
};

template<class Gear, unsigned N>
struct split_gear {
    static_assert(N != 0, "Unable to divide by zero");
    using type = ShipGear<Gear::cannons / N, Gear::masts / N, Gear::oars / N>;
};

template<class Gear>
class Squad {
    public:
    using gear_type = Gear;
    static gear_type gear;

    // We don't want the default constructor to be explicit
    constexpr Squad() noexcept : Squad(1) { }
    constexpr explicit Squad(const unsigned qty) noexcept : qty{qty} { }
    constexpr Squad(const Squad<Gear> &other) noexcept = default;

    // C++14 will require to state constexpr and const on methods separately
    // instead of assuming that constexpr method is const (as in C++11)
    constexpr unsigned get_count() const noexcept {
        return qty;
    }

    constexpr const Squad<Gear> operator+(const Squad<Gear> &other) const noexcept {
        // Explicit type annotations in return statements are unfortunately
        // required because the relevant constructor is explicit
        return Squad<Gear>{qty + other.qty};
    }

    constexpr const Squad<Gear> operator-(const Squad<Gear> &other) const noexcept {
        return Squad<Gear>{ship_utils::safe_sub(qty, other.qty)};
    }

    constexpr const Squad<Gear> operator*(int count) const noexcept {
        return Squad<Gear>{ship_utils::max0(static_cast<int>(qty) * count)};
    }
    
    constexpr const Squad<Gear> operator/(int count) const noexcept {
        return Squad<Gear>{ship_utils::max0(static_cast<int>(qty) / count)};
    }

    // The cost of copying Squad<Gear> is negligible, but such an implementation
    // allows to mark operator+, operator-, operator* and operator/ as constexpr.
    // Moreover, optimizing compilers should optimize this
    Squad<Gear>& operator+=(const Squad<Gear> &other) noexcept {
        return *this = *this + other;
    }

    Squad<Gear>& operator-=(const Squad<Gear> &other) noexcept {
        return *this = *this - other;
    }

    Squad<Gear>& operator*=(int count) noexcept {
        return *this = *this * count;
    }

    Squad<Gear>& operator/=(int count) noexcept {
        return *this = *this / count;
    }

    constexpr bool operator==(const Squad<Gear> &other) const noexcept {
        return qty == other.qty;
    }

    // Will be called iff Gear != OtherGear
    template<class OtherGear>
    constexpr bool operator==(const Squad<OtherGear>&) const noexcept {
        return Gear::cannons == OtherGear::cannons;
    }

    constexpr bool operator!=(const Squad<Gear> &other) const noexcept {
        return qty != other.qty;
    }

    // Will be called iff Gear != OtherGear
    template<class OtherGear>
    constexpr bool operator!=(const Squad<OtherGear>&) const noexcept {
        return Gear::cannons != OtherGear::cannons;
    }

    constexpr bool operator<(const Squad<Gear> &other) const noexcept {
        return qty < other.qty;
    }

    // Will be called iff Gear != OtherGear
    template<class OtherGear>
    constexpr bool operator<(const Squad<OtherGear>&) const noexcept {
        return Gear::cannons < OtherGear::cannons;
    }

    constexpr bool operator<=(const Squad<Gear> &other) const noexcept {
        return qty <= other.qty;
    }

    // Will be called iff Gear != OtherGear
    template<class OtherGear>
    constexpr bool operator<=(const Squad<OtherGear>&) const noexcept {
        return Gear::cannons <= OtherGear::cannons;
    }

    constexpr bool operator>(const Squad<Gear> &other) const noexcept {
        return qty > other.qty;
    }

    // Will be called iff Gear != OtherGear
    template<class OtherGear>
    constexpr bool operator>(const Squad<OtherGear>&) const noexcept {
        return Gear::cannons > OtherGear::cannons;
    }

    constexpr bool operator>=(const Squad<Gear> &other) const noexcept {
        return qty >= other.qty;
    }

    // Will be called iff Gear != OtherGear
    template<class OtherGear>
    constexpr bool operator>=(const Squad<OtherGear>&) const noexcept {
        return Gear::cannons >= OtherGear::cannons;
    }

    private:
    unsigned qty;
};

// Definitions of static data members of template classes are not subject to the
// One Definition Rule.
template<class Gear>
typename Squad<Gear>::gear_type Squad<Gear>::gear{};

template<class Gear, class OtherGear>
constexpr auto join_ships(const Squad<Gear> &s1, const Squad<OtherGear> &s2) 
    noexcept -> const Squad<typename add_gear<Gear, OtherGear>::type> {
    using namespace ship_utils;
    using JointGear = typename add_gear<Gear, OtherGear>::type;

    // If e.g. Gear::cannons + OtherGear::cannons is zero, then the respective
    // safe_avg is infinity (i.e. the largest representable value), so min
    // works properly
    return Squad<JointGear>{min(
        safe_avg(Gear::cannons,      s1.get_count(),
                 OtherGear::cannons, s2.get_count()),
        safe_avg(Gear::masts,        s1.get_count(),
                 OtherGear::masts,   s2.get_count()),
        safe_avg(Gear::oars,         s1.get_count(),
                 OtherGear::oars,    s2.get_count()),
        // The next line is important if the pirates' ships are primitive,
        // i.e. without cannons, masts or oars
        s1.get_count() + s2.get_count()
    )};
}

template<class Gear>
constexpr auto split_ships(const Squad<Gear> &s) noexcept
    -> const Squad<typename split_gear<Gear, 2>::type> {
    return Squad<typename split_gear<Gear, 2>::type>{s.get_count()};
}

namespace ship_utils {
    // Yet another auxiliary template, used in expected_booty
    // Basically it performs the comparison and stores the losing squad
    // (tuples compare lexicographically)
    template<class Gear, class OtherGear,
        bool FirstLoses = to_tuple<Gear>() < to_tuple<OtherGear>()>
    struct compare_squads {
        using result_type = Squad<Gear>;
        const Squad<Gear> &result;
        constexpr compare_squads(const Squad<Gear> &lhs, const Squad<OtherGear>&)
            noexcept : result(lhs) { }
    };

    template<class Gear, class OtherGear>
    struct compare_squads<Gear, OtherGear, false> {
        using result_type = Squad<OtherGear>;
        const Squad<OtherGear> &result;
        constexpr compare_squads(const Squad<Gear>&, const Squad<OtherGear> &rhs)
            noexcept : result(rhs) { }
    };
}

template<class Gear, class OtherGear>
constexpr const typename ship_utils::compare_squads<Gear, OtherGear>::result_type
expected_booty(const Squad<Gear> &s1, const Squad<OtherGear> &s2) noexcept {
    return ship_utils::compare_squads<Gear, OtherGear>{s1, s2}.result;
}

template<class Gear>
constexpr const Squad<Gear> operator*(int count, const Squad<Gear> &rhs) noexcept {
    return rhs * count;
}

template<class Gear>
std::ostream& operator<<(std::ostream &str, const Squad<Gear> &squad) {
    // Note: the Standard explicitly states, that operator<< called with integral
    // types receives them by value, so passing Gear::cannons etc. does not
    // mean that those static data members are being odr-used
    str << "Ships: " << squad.get_count() << "; ";
    str << "Ship gear: ";
    str << "Cannons: " << Gear::cannons << ", ";
    str << "Masts: " << Gear::masts << ", ";
    str << "Oars: " << Gear::oars;
    return str;
}

#endif // _SHIPWRECK_HH
