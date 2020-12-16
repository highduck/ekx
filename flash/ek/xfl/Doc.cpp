#include "Doc.hpp"

#include "parsing/doc_parser.hpp"

namespace ek::xfl {

const char* findLastOf(const std::string& str, char ch) {
    int sz = static_cast<int>(str.size());
    for (int i = sz - 1; i >= 0; --i) {
        if (str[i] == ch) {
            return str.c_str() + i;
        }
    }
    return nullptr;
}

Doc::Doc(std::unique_ptr<File> root) {
    doc_parser parser{*this, std::move(root)};
    parser.load();
}

Doc::Doc(const path_t& path) :
        Doc{load_flash_archive(path)} {

}

const element_t* Doc::find(const std::string& name,
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

const element_t* Doc::findLinkage(const std::string& className, element_type type) const {
    for (const auto& s: library) {
        if (s.item.linkageClassName == className &&
            (type == element_type::unknown || s.elementType == type)) {
            return &s;
        }
    }
    return nullptr;
}

}
