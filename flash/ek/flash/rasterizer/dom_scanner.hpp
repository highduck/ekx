#pragma once

#include "shape_processor.hpp"
#include "transform_model.hpp"
#include <vector>
#include <string>

namespace ek::flash {

class flash_doc;

struct element_t;

class dom_scanner {
public:
    std::string name;
    shape_processor output{};

    explicit dom_scanner(const flash_doc& doc);

    void reset();

    void scan(const element_t& element);

    void scan_group(const element_t& element);

    void scan_shape(const element_t& element);

    void scan_instance(const element_t& element, element_type type);

    void scan_symbol_item(const element_t& element);
private:
    const flash_doc& doc_;

    std::vector<transform_model> stack_;

    void push_transform(const element_t& el);

    void pop_transform();
};

rect_f estimate_bounds(const flash_doc& doc, const std::vector<element_t>& elements);

}
