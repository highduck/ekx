#pragma once

#include "types.hpp"
#include <memory>
#include <map>
#include <ek/ds/Array.hpp>

namespace pugi {
class xml_document;
}

namespace ek::xfl {

class File {
public:
    virtual ~File() = 0;

    [[nodiscard]] virtual pugi::xml_document* xml() const = 0;

    [[nodiscard]] virtual const std::string& content() const = 0;

    [[nodiscard]] virtual const File* open(const char* rpath) const = 0;

    static std::unique_ptr<File> load(const char* path);
};

class Doc {
public:
    Doc() = default;

    explicit Doc(std::unique_ptr<File> root);

    explicit Doc(const char* path);

    [[nodiscard]] const Element* find(const std::string& name,
                                      ElementType type = ElementType::unknown,
                                      bool ignoreFolders = false) const;

    [[nodiscard]] const Element* findLinkage(const std::string& className,
                                             ElementType type = ElementType::unknown) const;

public:
    DocInfo info;
    Array<FolderItem> folders{};
    Array<Element> library{};
    // order is important
    std::map<std::string, std::string> scenes;
private:
    std::unique_ptr<File> root_;
};

}