#include "sg_data.h"

#include <ek/serialize/streams.hpp>

namespace ek {

const sg_node_data* sg_file::get(const std::string& library_name) const {
    for (auto& child : library.children) {
        if (child.libraryName == library_name) {
            return &child;
        }
    }
    return nullptr;
}

}