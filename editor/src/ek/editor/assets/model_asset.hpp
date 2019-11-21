#pragma once

#include "editor_asset.hpp"

namespace ek {

class model_asset_t : public editor_asset_t {
public:
    explicit model_asset_t(std::string path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void load() override;

    void unload() override;

    void gui() override;

    void export_() override;

    void save() override;

    void export_meta(ek::output_memory_stream& output) override;

};

}