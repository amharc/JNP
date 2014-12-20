#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "strset.h"
#include "strsetconst.h"

using stringset = std::set<std::string>;

namespace {
    using stringsetmap = std::map<unsigned long, stringset>;

#ifdef DEBUG
    const bool debug = true;
#else
    const bool debug = false;
#endif

    // In order to prevent the static initialization order fiasco
    stringsetmap& stringsets() {
        static stringsetmap *object = new stringsetmap();
        return *object;
    }

    bool strset_modifiable(unsigned long id) {
        return id != strset42;
    }

    // Returns a prettyprinted name of the set identified by id
    std::string strset_name(unsigned long id) {
        if(id == strset42)
            return "the Set 42";
        else
            return "set " + std::to_string(id);
    }

    // Helper functions used to print debug information
    void _debug_print(const std::string&&) { }

    template<class X>
    void _debug_print(const std::string&&, const X &x) {
        std::clog << x;
    }

    template<class X, class... T>
    void _debug_print(const std::string &&sep, const X &x, const T&... rest) {
        std::clog << x << sep;
        _debug_print(std::forward<const std::string>(sep), rest...);
    }

    // Function to output debug information about function calls
    template<class... T>
    void debug_call(const std::string &function, const T&... arguments) {
        std::ios_base::Init init;
        std::clog << function << "(";
        _debug_print(", ", arguments...);
        std::clog << ")" << std::endl;
    }

    // Function to output debug information during the execution
    template<class... T>
    void debug_msg(const std::string &function, const T&... message) {
        std::ios_base::Init init;
        std::clog << function << ": ";
        _debug_print(" ", message...);
        std::clog << std::endl;
    }

    // Specialized functions generating uniform debugging informations
    void debug_doesnotexist(const std::string &function,
                            const unsigned long &id) {
        debug_msg(function, strset_name(id), "does not exist");
    }

    void debug_nullstring(const std::string &function) {
        debug_msg(function, "null value provided");
    }

    std::string quote(const char *txt) {
        if(txt == NULL)
            return "null";
        else
            return '"' + std::string{txt} + '"';
    }
}

// Creates a new set. The given ids are always greater than 0,
// so if strset42 is 0 (e.g. before it was initialized), no set is
// unmodifiable
unsigned long strset_new() {
    if(debug)
        debug_call(__func__);

    unsigned long id;

    if(stringsets().empty())
        id = 1;
    else
        id = stringsets().rbegin()->first + 1;

    if(debug && id == 0)
        debug_msg(__func__, "an overflow in the set id has occured"
                            "behaviour from now on is undefined");

    stringsets()[id] = stringset{};

    if(debug)
        debug_msg(__func__, strset_name(id), "created");
    return id;
}

void strset_delete(unsigned long id) {
    if(debug)
        debug_call(__func__, strset_name(id));

    auto iterator = stringsets().find(id);

    if(iterator == stringsets().end()) {
        if(debug)
            debug_doesnotexist(__func__, id);
        return;
    }

    if(!strset_modifiable(id)) {
        if(debug)
            debug_msg(__func__, "attempt to delete", strset_name(id));
        return;
    }

    stringsets().erase(iterator);
    if(debug)
        debug_msg(__func__, strset_name(id), "deleted");
}

size_t strset_size(unsigned long id) {
    if(debug)
        debug_call(__func__, strset_name(id));

    auto iterator = stringsets().find(id);

    if(iterator == stringsets().end()) {
        if(debug)
            debug_doesnotexist(__func__, id);
        return 0;
    }

    const stringset &set = iterator->second;
    size_t size = set.size();

    if(debug)
        debug_msg(__func__, strset_name(id), "contains", size, "element(s)");
    return size;
}

void strset_insert(unsigned long id, const char *value) {
    if(debug)
        debug_call(__func__, strset_name(id), quote(value));

    if(debug && value == NULL) {
        debug_nullstring(__func__);
        return;
    }

    auto iterator = stringsets().find(id);

    if(iterator == stringsets().end()) {
        if(debug)
            debug_doesnotexist(__func__, id);
        return;
    }

    stringset &set = iterator->second;

    if(!strset_modifiable(id)) {
        if(debug)
            debug_msg(__func__, "attempt to insert into", strset_name(id));
        return;
    }

    if(set.count(value) > 0) {
        if(debug)
            debug_msg(__func__, strset_name(id), "element", quote(value),
                      "is already present");
        return;
    }

    set.emplace(value);

    if(debug)
        debug_msg(__func__, "element", value, "inserted into", strset_name(id));
}

void strset_remove(unsigned long id, const char *value) {
    if(debug)
        debug_call(__func__, strset_name(id), quote(value));

    if(debug && value == NULL) {
        debug_nullstring(__func__);
        return;
    }

    auto iterator = stringsets().find(id);

    if(iterator == stringsets().end()) {
        if(debug)
            debug_doesnotexist(__func__, id);
        return;
    }

    stringset &set = iterator->second;

    if(!strset_modifiable(id)) {
        if(debug)
            debug_msg(__func__, "attempt to insert into",
                      strset_name(id));
        return;
    }

    if(set.count(value) == 0) {
        if(debug)
            debug_msg(__func__, strset_name(id),
                      "does not contain element", quote(value));
        return;
    }

    set.erase(value);

    if(debug)
        debug_msg(__func__, "element", quote(value), "removed from",
                  strset_name(id));
}

int strset_test(unsigned long id, const char *value) {
    if(debug)
        debug_call(__func__, strset_name(id), quote(value));

    if(debug && value == NULL) {
        debug_nullstring(__func__);
        return 0;
    }

    auto iterator = stringsets().find(id);

    if(iterator == stringsets().end()) {
        if(debug)
            debug_doesnotexist(__func__, id);
        return 0;
    }

    const stringset &set = iterator->second;

    bool result = set.count(value) > 0;

    if(debug) {
        if(result)
            debug_msg(__func__, strset_name(id),
                      "contains element", quote(value));
        else
            debug_msg(__func__, strset_name(id),
                      "does not contain element", quote(value));
    }

    return result;
}

void strset_clear(unsigned long id) {
    if(debug)
        debug_call(__func__, strset_name(id));

    auto iterator = stringsets().find(id);

    if(iterator == stringsets().end()) {
        if(debug)
            debug_doesnotexist(__func__, id);
        return;
    }

    stringset &set = iterator->second;

    if(!strset_modifiable(id)) {
        if(debug)
            debug_msg(__func__, "attempt to clear", strset_name(id));
        return;
    }

    set.clear();

    if(debug)
        debug_msg(__func__, strset_name(id), "cleared");
}

int strset_comp(unsigned long id1, unsigned long id2) {
    if(debug)
        debug_call(__func__, strset_name(id1), strset_name(id2));

    static const stringset empty;

    auto iterator1 = stringsets().find(id1),
         iterator2 = stringsets().find(id2);

    if(debug && iterator1 == stringsets().end())
        debug_doesnotexist(__func__, id1);

    // Here empty has to be an existing object, because placing a temporary,
    // i.e. stringset{}, causes the copy constructor to be called if the ternary
    // operator's condition is true
    const stringset &s1 = iterator1 != stringsets().end()
                          ? iterator1->second : empty;

    if(debug && iterator2 == stringsets().end())
        debug_doesnotexist(__func__, id2);

    const stringset &s2 = iterator2 != stringsets().end()
                          ? iterator2->second : empty;

    auto it1 = s1.cbegin(), it2 = s2.cbegin();

    while(it1 != s1.cend() && it2 != s2.cend() && *it1 == *it2) {
        ++it1;
        ++it2;
    }

    int ret;

    if(it1 == s1.cend() && it2 == s2.cend()) // sets are equal
        ret = 0;
    else if(it1 == s1.cend())
        ret = -1;
    else if(it2 == s2.cend())
        ret = 1;
    else if(*it1 < *it2)
        ret = -1;
    else // *it1 > *it2
        ret = 1;

    if(debug)
        debug_msg(__func__, "result of comparing", strset_name(id1), "to",
                  strset_name(id2), "is", ret);

    return ret;
}
