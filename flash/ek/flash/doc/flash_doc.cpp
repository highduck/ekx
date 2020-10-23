#include "flash_doc.hpp"
#include "flash_archive.hpp"
#include "parsing/parsing.hpp"

#include <pugixml.hpp>
#include <ek/flash/doc/parsing/doc_parser.hpp>

namespace ek::flash {

const element_t* flash_doc::find(const std::string& name, element_type type) const {
    for (const auto& s: library) {
        if (s.item.name == name && s.elementType == type) {
            return &s;
        }
    }
    return nullptr;
}

const element_t* flash_doc::find_linkage(const std::string& linkage) const {
    for (const auto& s: library) {
        if (s.item.linkageClassName == linkage) {
            return &s;
        }
    }
    return nullptr;
}

flash_doc::flash_doc(std::unique_ptr<basic_entry> root) {
    doc_parser parser{*this, std::move(root)};
    parser.load();
}

flash_doc::flash_doc(const path_t& path) :
        flash_doc{load_flash_archive(path)} {

}

}
