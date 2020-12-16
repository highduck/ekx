#include "Doc.hpp"

#include "parsing/DocParser.hpp"

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
    DocParser parser{*this, std::move(root)};
    parser.load();
}

Doc::Doc(const path_t& path) : Doc{File::load(path)} {
}

const Element* Doc::find(const std::string& name,
                         ElementType type,
                         bool ignoreFolders) const {
    for (const auto& s: library) {
        if (type == ElementType::unknown || s.elementType == type) {
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

const Element* Doc::findLinkage(const std::string& className, ElementType type) const {
    for (const auto& s: library) {
        if (s.item.linkageClassName == className &&
            (type == ElementType::unknown || s.elementType == type)) {
            return &s;
        }
    }
    return nullptr;
}

}
