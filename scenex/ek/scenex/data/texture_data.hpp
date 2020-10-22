#pragma once

#include <string>
#include <ek/serialize/serialize.hpp>

namespace ek {

struct texture_data_t {
    std::string texture_type;
    std::vector<std::string> images;

    template<typename S>
    void serialize(IO<S>& io) {
        io(texture_type, images);
    }
};

}

