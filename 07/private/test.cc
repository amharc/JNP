#include "fun_tree.h"
#include "fun_tree.h"
#include "fun_tree.h"
#include <functional>
#include <complex>
#include <iostream>
#include "fun_tree.h"
#include "fun_tree.h"
#include "fun_tree.h"

using namespace std;

float unop(int x) {
    return sqrt(x);
}

complex<double> binop(complex<double> acc, float mul) {
    return acc * complex<double>{mul};
}

struct Comparator {
    template<class U, class V>
    auto operator()(const U &u, const V &v) -> decltype(u < v) {
        return u < v;
    }
    static Comparator construct() {
        return {42, 42};
    }
    private:
    Comparator(int, int) { }
};

template<class T>
T id(const T &t) { return t; }

template<class T, class U>
T const_(T t, U) { return t; }

struct NonAssignable {
    int val;
    explicit NonAssignable(int val) : val(val) { }
    NonAssignable(const NonAssignable&) = default;
    NonAssignable& operator=(const NonAssignable&) = delete;
    NonAssignable& operator=(NonAssignable&&) = delete;
    NonAssignable operator+(const NonAssignable &other) const {
        return NonAssignable(val + other.val);
    }

    bool operator<(const NonAssignable &that) const {
        return val < that.val;
    }

    friend ostream& operator<<(ostream &str, const NonAssignable &na) {
        return str << na.val;
    }
};

struct CopyAssignable {
    int val;
    explicit CopyAssignable(int val) : val(val) { }
    CopyAssignable(const CopyAssignable &that) {
        val = that.val;
    }
    CopyAssignable& operator=(const CopyAssignable&) = default;
    CopyAssignable operator+(const CopyAssignable &other) const {
        return CopyAssignable(val + other.val);
    }

    bool operator<(const CopyAssignable &that) const {
        return val < that.val;
    }

    friend ostream& operator<<(ostream &str, const CopyAssignable &na) {
        return str << na.val;
    }
};

struct MoveAssignable {
    int val;
    explicit MoveAssignable(int val) : val(val) { }
    MoveAssignable(const MoveAssignable&) = default;
    MoveAssignable& operator=(const MoveAssignable&) = delete;
    MoveAssignable& operator=(MoveAssignable&&) = default;
    MoveAssignable operator+(const MoveAssignable &other) const {
        return MoveAssignable(val + other.val);
    }

    bool operator<(const MoveAssignable &that) const {
        return val < that.val;
    }

    friend ostream& operator<<(ostream &str, const MoveAssignable &na) {
        return str << na.val;
    }
};

struct BothAssignable {
    int val;
    BothAssignable(int val) : val(val) { }
    BothAssignable(const BothAssignable &that) {
        val = that.val;
        cout << "COPY CTOR :(" << endl;
    }
    BothAssignable(BothAssignable &&that) {
        val = that.val;
        cout << "move ctor :)" << endl;
    }

    BothAssignable& operator=(const BothAssignable &that) {
        val = that.val;
        cout << "COPY ASSIGNMENT :(" << endl;
        return *this;
    }
    BothAssignable& operator=(BothAssignable &&that) {
        val = that.val;
        cout << "move assignment :)" << endl;
        return *this;
    }
    BothAssignable operator+(BothAssignable &&other) const {
        return BothAssignable(val + other.val);
    }
    BothAssignable operator+(const BothAssignable &other) const {
        return BothAssignable(val + other.val);
    }

    bool operator<(const BothAssignable &that) const {
        return val < that.val;
    }

    friend ostream& operator<<(ostream &str, const BothAssignable &na) {
        return str << na.val;
    }
};

template class FunTree<int>;
template class FunTree<NonAssignable>;


struct Print {
    template<class A>
    void operator()(const A &a) {
        std::cerr << a << std::endl;
    }
};

struct Plus {
    static int counter;

    template<class A>
    auto operator()(A &&a) -> A {
        counter++;
        return a;
    }

    template<class A, class... As>
    auto operator()(A &&a, As&&... as)
        -> decltype(a + operator()(std::forward<As>(as)...)) {
        return a + operator()(std::forward<As>(as)...);
    }
};

template<class T>
void assignTest() {
    FunTree<T> na;
    na.insert(T(32));
    na.insert(T(87));
    cout << na.fold(std::plus<T>(), T(0)) << endl;
    cout << "Now it's ok to copy: " << endl;
    na.apply(id<T>);
    cout << "And now it's not" << endl;
    na.apply(Plus{});
}

int Plus::counter = 0;

struct First {
    template<class A, class B>
    A operator()(A &&a, B&&) {
        return a;
    }
};

template<class T>
class Dollar {
    const T val;
    public:
    template<class... Args>
    explicit Dollar(Args&&... args) : val(forward<Args>(args)...) { }
    Dollar(const Dollar&) = default;
    Dollar(Dollar&&) = default;
    template<class A>
    auto operator()(A &&a) -> decltype(a(val)) {
        return a(val);
    }
};

int printer1(int a) {
    cout << "Function 1: " << a << endl;
    return a;
}

int printer2(int a) {
    cout << "Function 2: " << a << endl;
    return a + 2;
}

struct FunComp {
    template<class U, class V>
    bool operator()(U &&u, V &&v) {
        return u(42) < v(42);
    }
};

template<class T>
Dollar<T> dollar(const T &arg) {
    return Dollar<T>{arg};
}

void dollarTest() {
    FunTree<std::function<int(int)>> tree;
    tree.insert(printer1, FunComp{});
    tree.insert(printer2, FunComp{});
    tree.apply(dollar(100));
}

int main() {
    FunTree<int> a;
    a.insert(25);
    a.insert(81);
    std::function<complex<double>(int)> f = a.fun(unop, binop);
    cout << f(1) << endl;
    a.apply([](int a) { return std::to_string(a); });
    auto binop2 = [](int a, float b) -> float { return a * b; };
    cout << a.fun(unop, binop)(42) << endl;
    cout << a.fun(&unop, &binop)(42) << endl;
    cout << a.fun(unop, binop2)(42) << endl;
    cout << a.fun([](long long a) { return sqrt(a); }, [](float b, double x) { return b * x; })(4) << endl;

    cout << std::bind(id<function<int(int)>>, [](int a) { return 2 * a; })()(42) << endl;
    auto res = a.fun(id<int>, const_<int, int>);
    cout << res(42) << endl;
    cout << std::bind(id<function<int(int)>>, res)()(42) << endl;
    cout << std::bind(id<function<string(int)>>,
            a.fun(
                [](int a){ return to_string(a); },
                [](string w, int q) { return to_string(q) + w; }
            ))()(42.0) << endl;

    cout << a.fold(std::plus<int>(), 0) << endl;

    assignTest<NonAssignable>();
    assignTest<CopyAssignable>();
    assignTest<MoveAssignable>();
    assignTest<BothAssignable>();

    cout << FunTree<int>::postorder << a << endl;
    cerr << "This should be different: " << endl << a << endl;
    stringstream str;
//    str << a.fun(Plus{}, First{})(2, 3) << endl;
//    str << a.fun(Plus{}, First{})(1, 2, M_PI, 97) << endl;
    Plus pl;
    First fst;
    BothAssignable x = 42;
    cout << "Now it's ok to copy: " << endl;
//    cout << a.fun(pl, fst)(BothAssignable(2), BothAssignable(3), x, x, x) << endl;
    a.apply(Print{});
    pl(3, 4);
    pl(2, 3, 4);

    a.insert(42, Comparator::construct());

    FunTree<complex<double>> c;
    c.insert(complex<double>(42), [](const complex<double> &lhs, const complex<double> &rhs) { return abs(lhs) < abs(rhs); });
    auto tester = a.fun(Plus{}, First{});
    cout << tester(2, M_PI, -2, 0, 0) << endl;
    cout << tester("hello", " ", std::string{"world"}) << endl;
    auto _ = c.fold(First{}, dollarTest);

    dollarTest();
}
