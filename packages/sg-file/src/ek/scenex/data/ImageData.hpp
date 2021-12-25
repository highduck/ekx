#pragma once

#include <ek/serialize/serialize.hpp>
#include <ek/ds/String.hpp>
#include <ek/ds/Array.hpp>

namespace ek {

enum class ImageDataType : uint32_t {
    Normal = 0,
    CubeMap = 1
};

struct ImageData {
    ImageDataType type;
    uint32_t formatMask = 1;
    Array<String> images;

    template<typename S>
    void serialize(IO<S>& io) {
        io(type, formatMask, images);
    }
};

}

