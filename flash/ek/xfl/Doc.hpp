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
};

std::unique_ptr<File> load_flash_archive(const path_t& path);

class Doc {
public:
    Doc() = default;

    explicit Doc(std::unique_ptr<File> root);

    explicit Doc(const path_t& path);

    [[nodiscard]] const element_t* find(const std::string& name,
                                        element_type type = element_type::unknown,
                                        bool ignoreFolders = false) const;

    [[nodiscard]] const element_t* findLinkage(const std::string& className,
                                               element_type type = element_type::unknown) const;

public:
    document_info info;
    std::vector<folder_item> folders;
    std::vector<element_t> library;
    // order is important
    std::map<std::string, std::string> scenes;
private:
    std::unique_ptr<File> root_;
};

}