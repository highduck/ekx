#pragma once

#include <ek/flash/doc/flash_archive.hpp>
#include <ek/flash/doc/flash_doc.hpp>
#include "parsing.hpp"

namespace ek::flash {

class doc_parser {
public:
    flash_doc& doc;
    std::unique_ptr<basic_entry> root;

    doc_parser(flash_doc& doc_, std::unique_ptr<basic_entry> root_);

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

