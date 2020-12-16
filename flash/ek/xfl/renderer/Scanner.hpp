#pragma once

#include "ShapeProcessor.hpp"
#include <vector>
#include <string>

namespace ek::xfl {

class Doc;

struct Element;

class Scanner {
public:
    std::string name;
    ShapeProcessor output{};

    explicit Scanner(const Doc& doc);

    void reset();

    void scan(const Element& element);

    void scan_group(const Element& element);

    void scan_shape(const Element& element);

    void scan_instance(const Element& element, ElementType type);

    void scan_symbol_item(const Element& element);

    static rect_f getBounds(const Doc& doc, const std::vector<Element>& elements);

private:
    const Doc& doc_;

    std::vector<TransformModel> stack_;

    void push_transform(const Element& el);

    void pop_transform();
};

}
