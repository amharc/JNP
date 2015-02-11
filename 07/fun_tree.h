#ifndef _FUN_TREE_H
#define _FUN_TREE_H

#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

// General note:
// C++ lacks a real type inference and polymorphism, compared to the functional
// programming languages, therefore this code looks ugly when compared with its
// eg. Haskell equivalent.
//
// The point-free style is preferred where possible. Every method is generalized
// as much as possible, i.e. it's a function template. Moreover, arguments are
// received by universal reference where possible
template<class T>
class FunTree {
    public:
    using Comparator = std::function<bool(const T&, const T&)>;
    using Operator = std::function<void(const T&)>;
    using Predicate = std::function<bool(const T&)>;
    using UnaryOperator = std::function<T(const T&)>;
    using BinaryOperator = std::function<T(const T&, const T&)>;

    private:
    struct Node;
    using node_ptr = std::shared_ptr<const Node>;
    using val_ptr = std::shared_ptr<const T>;

    node_ptr root;
    static int print_xalloc; // An identifier used for IO manipulation

    struct Node {
        const val_ptr value;
        const node_ptr left, right;

        Node(val_ptr value, node_ptr left = {}, node_ptr right = {})
            : value{value}
            , left{left}
            , right{right}
            { }

        node_ptr replace_left(node_ptr new_left) const {
            return std::make_shared<Node>(value, new_left, right);
        }

        node_ptr replace_right(node_ptr new_right) const {
            return std::make_shared<Node>(value, left, new_right);
        }
    };

    template<class Comparator>
    node_ptr do_insert(node_ptr to, val_ptr value, Comparator &&cmp) const {
        if(!to)
            return std::make_shared<Node>(value);
        else if(cmp(*value, *to->value))
            return to->replace_left(do_insert(to->left, value, cmp));
        else
            return to->replace_right(do_insert(to->right, value, cmp));
    }

    template<class Val, class Comparator>
    node_ptr do_erase(node_ptr to, const Val &value, Comparator &&cmp) const {
        if(!to)
            return {};
        else if(cmp(value, *to->value))
            return to->replace_left(do_erase(to->left, value, cmp));
        else if(cmp(*to->value, value))
            return to->replace_right(do_erase(to->right, value, cmp));
        else
            return {};
    }

    template<class Val, class Comparator>
    bool do_find(node_ptr to, const Val &value, Comparator &&cmp) const {
        if(!to)
            return false;
        else if(cmp(value, *to->value))
            return do_find(to->left, value, cmp);
        else if(cmp(*to->value, value))
            return do_find(to->right, value, cmp);
        else
            return true;
    }

    // Templated functions composition helper
    // std::bind cannot be used because nothing is assumed about the arity of G
    // If f, g, c are instances of F, G, Compose<F, G> respectively,
    // then f(g(arguments)) is equivalent to c(arguments), whatever should
    // the arguments be (in particular: they are being perfectly forwarded)
    // Note: f and g are copied (as they have to), but nevertheless they are
    // allowed to exhibit side effects when called, so operator() here is not
    // marked as const
    template<class F, class G>
    class Compose {
        private:
        typename std::decay<F>::type f;
        typename std::decay<G>::type g;
        public:
        Compose(F &&f, G &&g) : f(std::forward<F>(f)), g(std::forward<G>(g)) { }

        template<class... Args>
        auto operator()(Args&&... args)
            -> decltype(f(g(std::forward<Args>(args)...))) {
            return f(g(std::forward<Args>(args)...));
        }
    };

    // Composes two functions. 
    template<class F, class G>
    static Compose<F, G> compose(F &&f, G &&g) {
        return {std::forward<F>(f), std::forward<G>(g)};
    }

    // A value memoizer class. It allows to perform any operation on the stored
    // value, even if its type does not allow assigment -- copy construction
    // is used instead. If U is assignable, no allocation overhead will take place
    // ane the usual assigment operator will be called
    template<class U,
        bool Assignable = std::is_copy_assignable<U>::value
            || std::is_move_assignable<U>::value,
        bool Moveable = std::is_move_constructible<U>::value>
    class Memoizer {
        private:
        std::unique_ptr<U> memory;
        public:
        Memoizer(const U &initial) : memory{new U(initial)} { }
        Memoizer(U &&initial) : memory{new U(std::move(initial))} { }

        operator U() && {
            return *memory;
        }

        // Performs value = op(value, args...)
        // (of course args are perfectly forwarded)
        template<class Op, class... Args>
        void apply(Op &op, Args&&... args) {
            memory = std::unique_ptr<U>{
                new U(op(*memory, std::forward<Args>(args)...))
            };
        }
    };

    // And a specialization used if U is assignable and move constructible
    template<class U>
    class Memoizer<U, true, true> {
        private:
        U value;
        public:
        Memoizer(const U &initial) : value(initial) { }
        Memoizer(U &&initial) : value(std::move(initial)) { }

        operator U() && {
            return std::move(value);
        }

        template<class Op, class... Args>
        void apply(Op &op, Args&&... args) {
            value = op(std::move(value), std::forward<Args>(args)...);
        }
    };

    // And a specialization used if U is assignable but not move constructible
    template<class U>
    class Memoizer<U, true, false> {
        private:
        U value;
        public:
        Memoizer(const U &initial) : value(initial) { }

        operator U() && {
            return value;
        }

        template<class Op, class... Args>
        void apply(Op &op, Args&&... args) {
            value = op(value, std::forward<Args>(args)...);
        }
    };

    // The traversal functions.
    // They are implemented as Rank2Types because the function provided
    // could receive T either by value, const reference or const rvalue
    // reference. Fn is received by universal reference and, as it is 
    // allowed to exhibit impure side effects it is not marked as const
    struct Inorder {
        template<class Fn>
        static void apply(node_ptr node, Fn &&run) {
            if(node) {
                apply(node->left, run);
                run(*node->value);
                apply(node->right, run);
            }
        }
    };

    struct Preorder {
        template<class Fn>
        static void apply(node_ptr node, Fn &&run) {
            if(node) {
                run(*node->value);
                apply(node->left, run);
                apply(node->right, run);
            }
        }
    };

    struct Postorder {
        template<class Fn>
        static void apply(node_ptr node, Fn &&run) {
            if(node) {
                apply(node->left, run);
                apply(node->right, run);
                run(*node->value);
            }
        }
    };

    // Value to be held in the stream, indicating which traversal should be used
    enum class Traversal
        { INORDER = 0
        , PREORDER = 1
        , POSTORDER = 2
        };
    public:
    template<class Comparator = std::less<T>>
    void insert(const T &element, Comparator &&cmp = Comparator{}) {
        root = do_insert(root, std::make_shared<T>(element), cmp);
    }

    template<class Comparator = std::less<T>>
    void insert(T &&element, Comparator &&cmp = Comparator{}) {
        root = do_insert(root, std::make_shared<T>(std::move(element)), cmp);
    }

    template<class Comparator = std::less<T>>
    bool find(const T &element, Comparator &&cmp = Comparator{}) const {
        return do_find(root, element, cmp);
    }

    template<class Comparator = std::less<T>>
    void erase(const T &element, Comparator &&cmp = Comparator{}) {
        root = do_erase(root, element, cmp);
    }

    static Inorder inorder() {
        return Inorder{};
    }

    static Preorder preorder() {
        return Preorder{};
    }

    static Postorder postorder() {
        return Postorder{};
    }

    // The following functions receive a pointer to a Traversal-generating
    // function, but ignore it -- it's the type that matters
    template<class Operation, class Traversal = Inorder>
    void apply(Operation &&operation,
            Traversal(*)() = nullptr) const {
        Traversal::apply(root, std::forward<Operation>(operation));
    }

    template<class Operation,
        class Res = decltype(std::declval<Operation>()(*root->value)),
        class ResTree = FunTree<Res>,
        class Traversal = Inorder,
        class Comparator = std::less<Res>>
    ResTree map(Operation &&operation,
            Traversal(*)() = nullptr,
            Comparator &&cmp = Comparator{}) const {
        ResTree result;
        // Unfortunately std::bind will not work here due to inability
        // to resolve the overloaded function type, so an eta-expansion
        // is needed (strikingly similiar to the dreaded monomorphism
        // restriction from the functional programming languages)
        auto inserter = [&](Res &&res) {
            result.insert(res, cmp);
        };
        Traversal::apply(root,
            compose(inserter, std::forward<Operation>(operation)));
        return result;
    }

    template<class Predicate,
        class Traversal = Inorder,
        class Comparator = std::less<T>>
    FunTree<T> filter(Predicate &&predicate, 
            Traversal(*)() = nullptr,
            Comparator &&cmp = Comparator{}) const {
        FunTree<T> result;
        Traversal::apply(root, [&](const T &val) {
            if(predicate(val))
                result.insert(val, cmp);
        });
        return result;
    }

    template<class BinaryOperation,
        class Val,
        class Traversal = Inorder,
        class Decayed = typename std::decay<Val>::type>
    Decayed fold(BinaryOperation &&operation,
            Val &&init,
            Traversal(*)() = nullptr) const {
        Memoizer<Decayed> result{std::forward<Val>(init)};
        // Yet another eta-expansion
        Traversal::apply(root, [&](const T &val) {
            result.apply(operation, val);
        });
        return std::move(result);
    }

    template<class F, class Op>
    auto fun(F &&f, Op &&op) const
        -> decltype(compose(std::bind(std::forward<Op>(op),
                std::placeholders::_1, *root->value), std::forward<F>(f))) {
        if(!root)
            throw std::logic_error{"Fun called on an empty tree!"};
        return compose(std::bind(std::forward<Op>(op),
                    std::placeholders::_1, *root->value), std::forward<F>(f));
    }

    friend std::ostream& operator<<(std::ostream &str, const decltype(inorder)&) {
        str.iword(print_xalloc) = static_cast<long>(Traversal::INORDER);
        return str;
    }

    friend std::ostream& operator<<(std::ostream &str, const decltype(preorder)&) {
        str.iword(print_xalloc) = static_cast<long>(Traversal::PREORDER);
        return str;
    }

    friend std::ostream& operator<<(std::ostream &str, const decltype(postorder)&) {
        str.iword(print_xalloc) = static_cast<long>(Traversal::POSTORDER);
        return str;
    }

    friend std::ostream& operator<<(std::ostream &str, const FunTree<T> &tree) {
        using stream = decltype(std::ref(str));
        auto print = [](stream s, const T &v) {
            return std::ref(s.get() << " " << v);
        };
        switch(static_cast<Traversal>(str.iword(print_xalloc))) {
        case Traversal::INORDER:
            return tree.fold(print, std::ref(str), inorder);
        case Traversal::PREORDER:
            return tree.fold(print, std::ref(str), preorder);
        case Traversal::POSTORDER:
            return tree.fold(print, std::ref(str), postorder);
        default:
            throw std::logic_error{"Invalid traversal provided"};
        }
    }
};

template<class T>
int FunTree<T>::print_xalloc = std::ios_base::xalloc();

#endif // _FUN_TREE_H
