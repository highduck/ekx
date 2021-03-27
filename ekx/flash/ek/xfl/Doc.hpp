#pragma once

#include "types.hpp"
#include <memory>
#include <map>
#include <vector>

namespace pugi {
class xml_document;
}

namespace ek::xfl {

class File {
public:
    virtual ~File() = 0;

    [[nodiscard]] virtual pugi::xml_document* xml() const = 0;

    [[nodiscard]] virtual const std::string& content() const = 0;

    [[nodiscard]] virtual const File* open(const path_t& rpath) const = 0;

    static std::unique_ptr<File> load(const path_t& path);
};

class Doc {
public:
    Doc() = default;

    explicit Doc(std::unique_ptr<File> root);

    explicit Doc(const path_t& path);

    [[nodiscard]] const Element* find(const std::string& name,
                                      ElementType type = ElementType::unknown,
                                      bool ignoreFolders = false) const;

    [[nodiscard]] const Element* findLinkage(const std::string& className,
                                             ElementType type = ElementType::unknown) const;

public:
    DocInfo info;
    std::vector<FolderItem> folders;
    std::vector<Element> library;
    // order is important
    std::map<std::string, std::string> scenes;
private:
    std::unique_ptr<File> root_;
};

}