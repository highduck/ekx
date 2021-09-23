#pragma once

#include "../Doc.hpp"
#include "parsing.hpp"

namespace ek::xfl {

class DocParser {
public:
    Doc& doc;
    std::unique_ptr<File> root;

    DocParser(Doc& doc_, std::unique_ptr<File> root_);

    void load();
    void parse(const xml_node& node, Element& r) const;
    static void parse(const xml_node& node, ShapeObject& r) ;
    void parse(const xml_node& node, FillStyle& r) const;
    void parse(const xml_node& node, StrokeStyle& r) const;
    void parse(const xml_node& node, Frame& r) const;
    void parse(const xml_node& node, Layer& r) const;
    void parse(const xml_node& node, Timeline& r) const;

    template<typename T>
    inline T read(const xml_node& node) const {
        T r;
        parse(node, r);
        return r;
    }
};

}

