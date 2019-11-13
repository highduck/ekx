#pragma once

#include <string>
#include <ek/serialize/serialize.hpp>

namespace scenex {

struct program_data_t {
    std::string vertex_layout;
    std::string vertex_shader;
    std::string fragment_shader;

    template<typename S>
    void serialize(ek::IO<S>& io) {
        io(vertex_layout, vertex_shader, fragment_shader);
    }
};

}

