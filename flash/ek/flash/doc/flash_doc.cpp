#include "flash_doc.hpp"
#include "flash_archive.hpp"
#include "parsing/parsing.hpp"

#include <pugixml.hpp>
#include <ek/flash/doc/parsing/doc_parser.hpp>

namespace ek::flash {

const char* findLastOf(const std::string& str, char ch) {
    int sz = static_cast<int>(str.size());
    for (int i = sz - 1; i >= 0; --i) {
        if (str[i] == ch) {
            return str.c_str() + i;
        }
    }
    return nullptr;
}

const element_t* flash_doc::find(const std::string& name,
                                 element_type type,
                                 bool ignoreFolders) const {
    for (const auto& s: library) {
        if (type == element_type::unknown || s.elementType == type) {
            if (s.item.name == name) {
                return &s;
            }
            if (ignoreFolders) {
                auto stripped = findLastOf(s.item.name, '/');
                if (stripped != nullptr && name == (stripped + 1)) {
                    return &s;
                }
            }
        }
    }
    return nullptr;
}


const element_t* flash_doc::find_linkage(const std::string& className, element_type type) const {
    for (const auto& s: library) {
        if (s.item.linkageClassName == className &&
            (type == element_type::unknown || s.elementType == type)) {
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
