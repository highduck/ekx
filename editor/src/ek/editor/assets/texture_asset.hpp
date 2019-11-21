#pragma once

#include "editor_asset.hpp"
#include <vector>
#include <ek/fs/path.hpp>

namespace ek {

class texture_asset_t : public editor_asset_t {
public:
    explicit texture_asset_t(std::string path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void load() override;

    void unload() override;

    void gui() override;

    void export_() override;

    void save() override;

    void export_meta(ek::output_memory_stream& output) override;

private:
    std::string texture_type_;
    std::vector<std::string> images_;
};

}