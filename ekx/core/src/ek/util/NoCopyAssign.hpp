#pragma once

namespace ek {

struct NoCopyAssign {
    NoCopyAssign() = default;

    ~NoCopyAssign() = default;

    NoCopyAssign(const NoCopyAssign& v) = delete;

    NoCopyAssign& operator=(const NoCopyAssign& v) = delete;

    NoCopyAssign(NoCopyAssign&& v) = delete;

    NoCopyAssign& operator=(NoCopyAssign&& v) = delete;
};

}