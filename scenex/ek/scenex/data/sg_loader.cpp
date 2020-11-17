#include "sg_factory.hpp"

#include <ek/serialize/streams.hpp>
#include <ek/scenex/2d/Atlas.hpp>
#include <ek/app/res.hpp>
#include <ek/util/logger.hpp>

namespace ek {

sg_file* sg_load(const std::vector<uint8_t>& buffer) {
    sg_file* sg = nullptr;

    if (!buffer.empty()) {
        input_memory_stream input{buffer.data(), buffer.size()};
        IO io{input};

        sg = new sg_file;
        io(*sg);
    } else {
        EK_ERROR("SCENE LOAD: empty buffer");
    }

    return sg;
}

}