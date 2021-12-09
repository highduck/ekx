#include "ExportItem.hpp"
#include "../xfl/types.hpp"

namespace ek::xfl {

ExportItem::~ExportItem() {
    for (auto ch : children) {
        delete ch;
    }
}

void ExportItem::add(ExportItem* item) {
    assert(item != nullptr);
    children.push_back(item);
    item->parent = this;
}

void ExportItem::append_to(ExportItem* parent_) {
    assert(parent == nullptr);
    if (parent_) {
        parent_->add(this);
    }
}

ExportItem* ExportItem::find_library_item(const String& libraryName) const {
    for (auto& child : children) {
        if (child->node.libraryName == libraryName) {
            return child;
        }
    }
    return nullptr;
}

void ExportItem::inc_ref(ExportItem& lib) {
    ++usage;
    if (!node.libraryName.empty()) {
        auto* dependency = lib.find_library_item(node.libraryName);
        if (dependency && dependency != this) {
            dependency->inc_ref(lib);
        }
    }
    // else it's something like dynamic_text?
//    else {
//        EK_WARN << ref->item.linkageClassName;
//    }
    for (auto* child : children) {
        child->inc_ref(lib);
    }
}

void ExportItem::update_scale(ExportItem& lib, const Matrix3x2f& parent_matrix) {
    if (!node.scaleGrid.empty()) {
        estimated_scale = 1.0f;
        return;
    }

    const auto global_matrix = parent_matrix * node.matrix;
    const auto scale = global_matrix.scale();
    const auto s = std::max(scale.x, scale.y);
    estimated_scale = std::max(s, estimated_scale);
    if (ref && ref->elementType == ElementType::bitmap_item) {
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