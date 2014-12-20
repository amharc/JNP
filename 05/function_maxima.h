#ifndef _FUNCTION_MAXIMA_H
#define _FUNCTION_MAXIMA_H

#include <exception>
#include <iterator>
#include <memory>
#include <set>

class InvalidArg : public std::exception {
    public:
    virtual const char* what() const noexcept override {
        return "InvalidArg";
    }
};

template<class A, class V>
class FunctionMaxima {
    public:
        class point_type;
    private:
        struct arg_compare;
        struct maxima_compare;
        using arg_ptr = std::shared_ptr<const A>;
        using value_ptr = std::shared_ptr<const V>;

        // values_set_type and maxima_set_type are multisets, but duplicates
        // may occur only for a very short time -- only while erasing or
        // inserting something -- due to the transaction-like nature of those
        // operations (it is crucial to perform insertion before erasure,
        // in order to provide strong exception guarantee)
        using values_set_type = std::multiset<point_type, arg_compare>;
        using maxima_set_type = std::multiset<point_type, maxima_compare>;

    public:
        using size_type = typename values_set_type::size_type;
        using iterator = typename values_set_type::const_iterator;
        using mx_iterator = typename maxima_set_type::const_iterator;

    private:
        values_set_type values;
        maxima_set_type maxima;

        // A hack used to temporarily pack a reference into shared_ptr
        // without copying or transfering ownership -- null_deleter is passed
        // as deleter to the constructor of shared_ptr. When the shared_ptr
        // is destroyed, the object referenced by this pointer will not be freed
        struct null_deleter {
            void operator()(const void*) const noexcept { }
        };

        // Returns a fictional point_type for an argument without value
        // Because arg_compare compares only arguments, such a point_type
        // may be used e.g. to find something in the values set.
        point_type make_null_value(arg_ptr arg) const noexcept {
            return {arg, {}};
        }

        // Returns an arg_ptr from a const reference. Because arg_ptr is a smart
        // pointer, it is necessary to specify that it should not delete the
        // value held, because it does not logically gain ownership of A,
        // contrary to the usual use of shared pointers.
        arg_ptr make_arg_ptr(const A &a) const {
            return {&a, null_deleter{}};
        }

        iterator find_by_argument(arg_ptr arg) const {
            return values.find(make_null_value(arg));
        }

        iterator find_by_argument(const A &a) const {
            return find_by_argument(make_arg_ptr(a));
        }

        // Helper functions returning successors and predecessors
        // in the domain. values.end() is used as a generic no-such-element
        // iterator (And because of that, those functions cannot be const)
        //
        // Because they may be used during the transactions when the mulisets
        // are not de facto sets, i.e. they have duplicate keys, those functions
        // have while loops in them, but all those loops will loop at most twice
        iterator successor(iterator iter) const {
            if(iter == values.end())
                return values.end();
            else {
                const auto &orig_arg = iter->arg();
                while(iter != values.end() && !(orig_arg < iter->arg()))
                    ++iter;

                return iter;
            }
        }

        iterator predecessor(iterator iter) const {
            if(iter == values.end())
                return values.end();
            else {
                const auto &orig_arg = iter->arg();
                while(iter != values.begin() && !(iter->arg() < orig_arg))
                    --iter;

                if(iter->arg() < orig_arg)
                    return iter;
                else
                    return values.end();
            }
        }

        // Comparison helpers
        struct arg_compare {
            bool operator()(const point_type &lhs, const point_type &rhs) const {
                return lhs.arg() < rhs.arg();
            }
        };

        struct maxima_compare {
            bool operator()(const point_type &lhs, const point_type &rhs) const {
                // Compare the values
                if(lhs.value() < rhs.value())
                    return false;

                if(rhs.value() < lhs.value())
                    return true;

                // In the event of a tie, compare the arguments
                return lhs.arg() < rhs.arg();
            }
        };

        // maxima_deleter, maxima_updater and value_updater are helper classes
        // used to enforce a transaction-like behaviour of some operations on
        // FunctionMaxima. In order to correctly use them, the user should:
        // 1) Create the respective helper
        // 2) Perform other unsafe operations (i.e. that operations which may
        //    throw an exception)
        // 3) When the transaction should be commited (i.e. no exceptions will
        //    be thrown anymore), it should call commit
        //
        // Responsibilities:
        // maxima_deleter -- is responsible for safe deletion of a maximum
        // maxima_updater -- is responsible for safe update of a maximum.
        //                   It will also delete the old maximum, but only
        //                   if the value associated with the argument is not
        //                   changed. In other cases caller should either
        //                   construct a maxima_deleter directly or using a
        //                   proxy such as value_updater
        // value_updater  -- is responsible for safe insertion of a value.
        //                   It also removes the old value associated with the
        //                   provided argument, if any. If a value has been
        //                   associated with the provided argument before,
        //                   and an assignment of a different one (in terms of
        //                   operator<) is performed, it is also responsible for
        //                   deletion of the maximum
        //
        // Common interface:
        // fm -- pointer to the FunctionMaxima<A, V>
        // all those classes are non-copyable and non-moveable
        class maxima_deleter {
            public:
                maxima_deleter(FunctionMaxima<A, V> *fm, iterator my)
                    : fm(fm) {
                    if(my == fm->values.end())
                        iter = fm->maxima.end();
                    else
                        iter = fm->maxima.find(*my);
                }

                maxima_deleter(const maxima_deleter&) = delete;
                maxima_deleter(maxima_deleter&&) = delete;
                maxima_deleter& operator=(const maxima_deleter&) = delete;
                maxima_deleter& operator=(maxima_deleter&&) = delete;

                // All work is done in the constructor and commit()
                ~maxima_deleter() = default;

                void commit() noexcept {
                    if(iter != fm->maxima.end()) {
                        fm->maxima.erase(iter);
                    }
                }
            private:
                FunctionMaxima<A, V> *fm;
                mx_iterator iter;
        };

        class maxima_updater {
            public:
                // left, my, right -- the iterators to the arguments: immediately
                // preceding the candidate for maximum, the candidate itself and
                // the immediately succeeding argument
                maxima_updater(FunctionMaxima<A, V> *fm, iterator left,
                           iterator my, iterator right) : fm{fm}, left{left},
                        my{my}, right{right}, commited{false} {
                    if(my == fm->values.end()) {
                        old_iter = iter = fm->maxima.end();
                        cond = false;
                    }
                    else {
                        cond = true;
                        old_iter = fm->maxima.find(*my);

                        if(left != fm->values.end())
                            cond &= !(my->value() < left->value());
                        if(right != fm->values.end())
                            cond &= !(my->value() < right->value());

                        if(cond)
                            iter = fm->maxima.emplace(*my);
                    }
                }

                maxima_updater(const maxima_updater&) = delete;
                maxima_updater(maxima_updater&&) = delete;
                maxima_updater& operator=(const maxima_updater&) = delete;
                maxima_updater& operator=(maxima_updater&&) = delete;

                ~maxima_updater() {
                    // Rollback
                    if(!commited && cond)
                        fm->maxima.erase(iter);
                }

                void commit() noexcept {
                    if(old_iter != fm->maxima.end())
                        fm->maxima.erase(old_iter);

                    commited = true;
                }

            private:
                FunctionMaxima<A, V> *fm;
                iterator left, my, right;
                mx_iterator old_iter, iter;
                bool commited, cond;
        };

        class value_updater {
            public:
                value_updater(FunctionMaxima<A, V> *fm, arg_ptr arg,
                    value_ptr value) : fm{fm}, old_iter{fm->find_by_argument(arg)},
                       commited{false} {

                    if(old_iter != fm->values.end() &&
                            ((old_iter->value() < *value) ||
                            (*value < old_iter->value()))) {
                        // We ought to delete the old maximum (as we are
                        // responsible for it)
                        deleter.reset(new maxima_deleter{fm, old_iter});
                    }

                    iter = fm->values.insert({arg, value});
                }

                value_updater(const value_updater&) = delete;
                value_updater(value_updater&&) = delete;
                value_updater& operator=(const value_updater&) = delete;
                value_updater& operator=(value_updater&&) = delete;

                iterator get_iterator() const noexcept {
                    return iter;
                }

                void commit() noexcept {
                    if(old_iter != fm->values.end())
                        fm->values.erase(old_iter);

                    if(deleter)
                        deleter->commit();

                    commited = true;
                }

                ~value_updater() {
                    // Rollback
                    if(!commited)
                        fm->values.erase(iter);
                }

            private:
                FunctionMaxima<A, V> *fm;
                iterator old_iter, iter;
                bool commited;
                // Used to conditionally hold a maxima_deleter
                std::unique_ptr<maxima_deleter> deleter;
        };

    public:

        class point_type {
            private:
                arg_ptr _arg;
                value_ptr _value;

                point_type(arg_ptr arg, value_ptr value) noexcept
                    : _arg(arg), _value(value) { }
            public:
                point_type(const point_type&) noexcept = default;
                point_type(point_type&&) noexcept = default;
                point_type& operator=(const point_type&) noexcept = default;
                point_type& operator=(point_type&&) noexcept = default;

                const A& arg() const noexcept {
                    return *_arg;
                }

                const V& value() const noexcept {
                    return *_value;
                }

                friend class FunctionMaxima<A, V>;
        };

        FunctionMaxima() { }

        FunctionMaxima(const FunctionMaxima&) = default;

        FunctionMaxima(FunctionMaxima &&that) noexcept {
            std::swap(values, that.values);
            std::swap(maxima, that.maxima);
        }

        // A unifying assignment operator
        FunctionMaxima& operator=(FunctionMaxima that) noexcept {
            std::swap(values, that.values);
            std::swap(maxima, that.maxima);
            return *this;
        }

        const V& value_at(const A &a) const {
            auto it = find_by_argument(a);

            if(it == values.end())
                throw InvalidArg{};

            return it->value();
        }

        void set_value(const A &a, const V &v) {
            // Copy the arguments (it may throw)
            auto arg = std::make_shared<A>(a);
            auto value = std::make_shared<V>(v);
            
            // Find the required iterators (it may throw)
            value_updater inserter{this, arg, value};
            auto my = inserter.get_iterator();
            auto left1 = predecessor(my);
            auto left2 = predecessor(left1);
            auto right1 = successor(my);
            auto right2 = successor(right1);

            maxima_updater maxima_left{this, left2, left1, my};
            maxima_updater maxima_my{this, left1, my, right1};
            maxima_updater maxima_right{this, my, right1, right2};

            // Now entering the nonthrowing part
            inserter.commit();
            maxima_left.commit();
            maxima_my.commit();
            maxima_right.commit();
        }

        void erase(const A &a) {
            auto my = find_by_argument(a);
            if(my == values.end())
                return;

            auto left1 = predecessor(my);
            auto left2 = predecessor(left1);
            auto right1 = successor(my);
            auto right2 = successor(right1);

            maxima_updater maxima_left{this, left2, left1, right1};
            maxima_deleter maxima_my{this, my};
            maxima_updater maxima_right{this, left1, right1, right2};

            // Now entering the nonthrowing part
            values.erase(my);
            maxima_left.commit();
            maxima_my.commit();
            maxima_right.commit();
        }

        iterator begin() const noexcept {
            return values.cbegin();
        }

        iterator end() const noexcept {
            return values.cend();
        }

        iterator find(const A &a) const {
            return find_by_argument(a);
        }

        mx_iterator mx_begin() const noexcept {
            return maxima.cbegin();
        }

        mx_iterator mx_end() const noexcept {
            return maxima.cend();
        }

        size_type size() const noexcept {
            return values.size();
        }
};

#endif // _FUNCTION_MAXIMA_H
