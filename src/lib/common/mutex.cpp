// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "lib/common/mutex.hpp"

#ifdef FCPP_DISABLE_THREADS
//! @cond INTERNAL
namespace std {
    namespace this_thread {
        void yield() {}
    };
}
//! @endcond
#endif
