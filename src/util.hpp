
#pragma once

#include <cmath>
#include <cassert>

namespace tic_tac_toe {
    using U8 = unsigned char;
    static_assert(sizeof(U8) == 1);
    using U16 = unsigned short;
    static_assert(sizeof(U16) == 2);
    using U32 = unsigned;
    static_assert(sizeof(U32) == 4);
    using U64 = unsigned long long;
    static_assert(sizeof(U64) == 8);

namespace util {
    inline U32 min(U32 a, U32 b) {
        return a < b ? a : b;
    }

    inline U32 max(U32 a, U32 b) {
        return a > b ? a : b;
    }

    inline U8 random(U8 from, U8 till) {
        const U8 result = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) * (till - from) + from;
        assert(result >= from);
        assert(result < till);
        return result;
    }
} // namespace util
} // namespace tic_tac_toe
