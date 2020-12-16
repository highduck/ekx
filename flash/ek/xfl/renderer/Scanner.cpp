#include "Scanner.hpp"

#include <ek/xfl/Doc.hpp>

namespace ek::xfl {

Scanner::Scanner(const Doc& doc) : doc_{doc} {
    reset();
}

void Scanner::reset() {
    stack_.clear();
    stack_.emplace_back();
    output.reset();
}

void Scanner::scan_instance(const Element& element, ElementType type) {
    auto* s = doc_.find(element.libraryItemName, type);
    if (s) {
        push_transform(element);
        scan(*s);
        pop_transform();
    }
}

void Scanner::scan_group(const Element& element) {
    // ! Group Transformation is not applied !
    for (const auto& member : element.members) {
        scan(member);
    }
}

void Scanner::scan_shape(const Element& element) {
    push_transform(element);
    output.add(element, stack_.back());
    pop_transform();
}

void Scanner::scan_symbol_item(const Element& element) {
    const auto& layers = element.timeline.layers;
    const auto& end = layers.rend();
    for (auto it = layers.rbegin(); it != end; ++it) {
        const auto& layer = *it;
        if (layer.layerType == LayerType::normal) {
            if (!layer.frames.empty()) {
                for (const auto& el : layer.frames[0].elements) {
                    scan(el);
                }
            }
        }
    }
}

void Scanner::scan(const Element& element) {
    switch (element.elementType) {
        case ElementType::symbol_instance:
            scan_instance(element, ElementType::symbol_item);
            break;
        case ElementType::bitmap_instance:
            scan_instance(element, ElementType::bitmap_item);
            break;
        case ElementType::symbol_item:
        case ElementType::scene_timeline:
            scan_symbol_item(element);
            break;
        case ElementType::group:
            scan_group(element);
            break;
        case ElementType::shape:
        case ElementType::object_rectangle:
        case ElementType::object_oval:
        case ElementType::bitmap_item:
            scan_shape(element);
            break;
        default:
            break;
    }
}

void Scanner::push_transform(const Element& el) {
    stack_.push_back(stack_.back() * el.transform);
}

void Scanner::pop_transform() {
    stack_.pop_back();
}

rect_f Scanner::getBounds(const Doc& doc, const std::vector<Element>& elements) {
    Scanner scanner(doc);
    for (auto& el: elements) {
        scanner.scan(el);
    }
    return scanner.output.bounds.rect();
}

}