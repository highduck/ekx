#include "export_item.h"
#include <ek/flash/doc/element_types.h>

namespace ek::flash {

export_item_t::~export_item_t() {
    for (auto ch : children) {
        delete ch;
    }
}

void export_item_t::add(export_item_t* item) {
    assert(item != nullptr);
    children.push_back(item);
    item->parent = this;
}

void export_item_t::append_to(export_item_t* parent_) {
    assert(parent == nullptr);
    if (parent_) {
        parent_->add(this);
    }
}

export_item_t* export_item_t::find_library_item(const std::string& libraryName) const {
    for (auto& child : children) {
        if (child->node.libraryName == libraryName) {
            return child;
        }
    }
    return nullptr;
}

void export_item_t::inc_ref(export_item_t& lib) {
    ++usage;
    if (!node.libraryName.empty()) {
        auto* dependency = lib.find_library_item(node.libraryName);
        if (dependency && dependency != this) {
            dependency->inc_ref(lib);
        }
    }
    for (auto* child : children) {
        child->inc_ref(lib);
    }
}

void export_item_t::update_scale(export_item_t& lib, const matrix_2d& parent_matrix) {
    if (!node.scaleGrid.empty()) {
        estimated_scale = 1.0f;
        return;
    }

    const auto global_matrix = parent_matrix * node.matrix;
    const auto scale = global_matrix.scale();
    const auto s = std::max(scale.x, scale.y);
    estimated_scale = std::max(s, estimated_scale);
    if (ref && ref->elementType == element_type::bitmap_item) {
        max_abs_scale = 1.0f;
    }

    if (!node.libraryName.empty()) {
        auto* dependency = lib.find_library_item(node.libraryName);
        if (dependency && dependency != this) {
            dependency->update_scale(lib, global_matrix);
        }
    }
    for (auto& child : children) {
        child->update_scale(lib, global_matrix);
    }
}

}