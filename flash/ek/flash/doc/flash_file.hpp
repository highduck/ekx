#pragma once

#include "types.hpp"
#include <memory>

namespace ek::flash {

class basic_entry;

class flash_file {
public:
    flash_file() = default;

    explicit flash_file(std::unique_ptr<basic_entry> root);

    [[nodiscard]] const element_t* find(const std::string& name, element_type type) const;
    [[nodiscard]] const element_t* find_linkage(const std::string& linkage) const;

public:
    document_info info;
    std::vector<folder_item> folders;
    std::vector<element_t> library;
    std::vector<timeline_t> timelines;
private:
    std::unique_ptr<basic_entry> root_;
};

}