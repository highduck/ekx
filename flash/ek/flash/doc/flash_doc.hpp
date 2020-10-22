#pragma once

#include "types.hpp"
#include <memory>
#include <map>
#include <vector>

namespace ek::flash {

class basic_entry;

class flash_doc {
public:
    flash_doc() = default;

    explicit flash_doc(std::unique_ptr<basic_entry> root);
    explicit flash_doc(const path_t& path);

    [[nodiscard]] const element_t* find(const std::string& name, element_type type) const;
    [[nodiscard]] const element_t* find_linkage(const std::string& linkage) const;

public:
    document_info info;
    std::vector<folder_item> folders;
    std::vector<element_t> library;
    // order is important
    std::map<std::string, std::string> scenes;
private:
    std::unique_ptr<basic_entry> root_;
};

}