#pragma once

#include <string>
#include <ek/serialize/serialize.hpp>
#include <ek/ds/Array.hpp>

namespace ek {

struct texture_data_t {
    std::string texture_type;
    Array<std::string> images;

    template<typename S>
    void serialize(IO<S>& io) {
        io(texture_type, images);
    }
};

}

