#pragma once

#include <ek/xfl/Doc.hpp>
#include "parsing.hpp"

namespace ek::xfl {

class doc_parser {
public:
    Doc& doc;
    std::unique_ptr<File> root;

    doc_parser(Doc& doc_, std::unique_ptr<File> root_);

    void load();
    void parse(const xml_node& node, element_t& r) const;
    static void parse(const xml_node& node, shape_object_t& r) ;
    void parse(const xml_node& node, fill_style& r) const;
    void parse(const xml_node& node, stroke_style& r) const;
    void parse(const xml_node& node, frame_t& r) const;
    void parse(const xml_node& node, layer_t& r) const;
    void parse(const xml_node& node, timeline_t& r) const;

    template<typename T>
    inline T read(const xml_node& node) const {
        T r;
        parse(node, r);
        return r;
    }
};

}

