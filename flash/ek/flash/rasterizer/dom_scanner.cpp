#include "dom_scanner.hpp"

#include <ek/flash/doc/flash_file.hpp>

namespace ek::flash {

dom_scanner::dom_scanner(const flash_file& doc) : file_{doc} {
    reset();
}

void dom_scanner::reset() {
    stack_.clear();
    stack_.emplace_back();
    output.reset();
}

void dom_scanner::scan_symbol_instance(const element_t& element) {
    auto* s = file_.find(element.libraryItemName, element_type::symbol_item);
    if (s) {
        push_transform(element);
        scan(*s);
        pop_transform();
    }
}

void dom_scanner::scan_bitmap_instance(const element_t& element) {
    auto* s = file_.find(element.libraryItemName, element_type::bitmap_item);
    if (s) {
        push_transform(element);
        scan_bitmap_item(*s);
        pop_transform();
    }
}

void dom_scanner::scan_bitmap_item(const element_t& element) {
    push_transform(element);
    output.add(element, stack_.back());
    pop_transform();
}

void dom_scanner::scan_group(const element_t& element) {
    // ! Group Transformation is not applied !
    for (const auto& member : element.members) {
        scan(member);
    }
}

void dom_scanner::scan_shape(const element_t& element) {
    push_transform(element);
    output.add(element, stack_.back());
    pop_transform();
}

void dom_scanner::scan_symbol_item(const element_t& element) {
    const auto& layers = element.timeline.layers;
    const auto& end = layers.rend();
    for (auto it = layers.rbegin(); it != end; ++it) {
        const auto& layer = *it;
        if (layer.layerType == layer_type::normal) {
            if (!layer.frames.empty()) {
                for (const auto& el : layer.frames[0].elements) {
                    scan(el);
                }
            }
        }
    }
}

void dom_scanner::scan(const element_t& element) {
    switch (element.elementType) {
        case element_type::symbol_instance:
            scan_symbol_instance(element);
            break;
        case element_type::group:
            scan_group(element);
            break;
        case element_type::shape:
            scan_shape(element);
            break;
        case element_type::symbol_item:
            scan_symbol_item(element);
            break;
        case element_type::bitmap_instance:
            scan_bitmap_instance(element);
            break;
        case element_type::bitmap_item:
            scan_bitmap_item(element);
            break;
        default:
            break;
    }
}

void dom_scanner::push_transform(const element_t& el) {
    stack_.push_back(stack_.back() * transform_model{el.matrix, el.color, el.blend_mode});
}

void dom_scanner::pop_transform() {
    stack_.pop_back();
}

rect_f estimate_bounds(const flash_file& doc, const std::vector<element_t>& elements) {
    dom_scanner scanner(doc);
    for (auto& el: elements) {
        scanner.scan(el);
    }
    return scanner.output.bounds.rect();
}

}