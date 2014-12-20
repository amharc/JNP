#include <ios>
#include <iostream>
#include "strset.h"
#include "strsetconst.h"

namespace {
#ifdef DEBUG
    const bool debug = true;
#else
    const bool debug = false;
#endif

    // A helper function used to initialize strset42
    unsigned long strsetconst_init() {
        std::ios_base::Init init;

        if(debug)
            std::clog << __func__ << "()" << std::endl;

        // While the Set 42 is being initialized, the value of strset42 is 0,
        // by par. 3.6.2.2 of the C++ Standard
        unsigned long id = strset_new();
        strset_insert(id, "42");

        if(debug)
            std::clog << __func__ << ": finished. From now on, set " << id <<
                                     " is the Set 42" << std::endl;
        return id;
    }
}

const unsigned long strset42 = strsetconst_init();
