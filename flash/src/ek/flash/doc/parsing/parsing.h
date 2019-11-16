#pragma once

namespace pugi {

class xml_node;

class xml_document;

}

namespace ek::flash {

using xml_node = pugi::xml_node;
using xml_document = pugi::xml_document;

template<typename T>
T parse_xml_node(const xml_node& node) {
    T r;
    r << node;
    return r;
}

}


