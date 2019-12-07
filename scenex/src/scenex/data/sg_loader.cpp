#include "sg_factory.h"

#include <ek/serialize/streams.hpp>
#include <scenex/2d/atlas.hpp>
#include <platform/static_resources.hpp>
#include <ek/util/logger.hpp>

using namespace ek;

namespace scenex {

sg_file* sg_load(const std::vector<uint8_t>& buffer) {
    sg_file* sg = nullptr;

    if (!buffer.empty()) {
        input_memory_stream input{buffer.data(), buffer.size()};
        IO io{input};

        sg = new sg_file;
        io(*sg);
    }
    else {
        EK_ERROR << "SCENE LOAD: empty buffer";
    }

    return sg;
}

}