#include "sg_factory.h"

#include <ek/serialize/streams.hpp>
#include <scenex/2d/atlas.hpp>
#include <platform/static_resources.hpp>
#include <ek/logger.hpp>

using namespace ek;

namespace scenex {

sg_file* sg_load(const std::string& path) {
    sg_file* sg = nullptr;

    auto buffer = get_resource_content(path);
    if (buffer.empty()) {
        EK_ERROR << "SCENE resource not found: " << path;
    } else {
        input_memory_stream input{buffer.data(), buffer.size()};
        IO io{input};

        sg = new sg_file;
        io(*sg);
    }

    return sg;
}

}