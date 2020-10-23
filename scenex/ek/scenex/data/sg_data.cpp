#include "sg_data.hpp"

#include <ek/serialize/streams.hpp>

namespace ek {

const sg_node_data* sg_file::get(const std::string& library_name) const {
    for (auto& item : library) {
        if (item.libraryName == library_name) {
            return &item;
        }
    }
    return nullptr;
}

}