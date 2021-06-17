#pragma once

#include <string>
#include <ek/serialize/serialize.hpp>
#include <ek/serialize/stl/String.hpp>
#include <ek/ds/Array.hpp>

namespace ek {

enum class TextureDataType : uint32_t {
    Normal = 0,
    CubeMap = 1
};

struct TextureData {
    TextureDataType type;
    Array<std::string> images;

    template<typename S>
    void serialize(IO<S>& io) {
        io(type, images);
    }
};

}

