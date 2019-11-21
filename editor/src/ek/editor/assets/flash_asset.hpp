#pragma once

#include "editor_asset.hpp"
#include <ek/spritepack/atlas_declaration.hpp>
#include <ek/flash/doc/flash_archive.h>
#include <memory>

namespace ek {

class flash_asset_t : public editor_asset_t {
public:
    explicit flash_asset_t(std::string path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void load() override;

    void unload() override;

    void gui() override;

    void export_() override;

    void save() override;

    void export_meta(output_memory_stream& output) override;
private:
    atlas_decl_t atlas_decl_;
};

std::unique_ptr<flash::basic_entry> load_flash_archive(const path_t& path);

}

