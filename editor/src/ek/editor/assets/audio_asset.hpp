#pragma once

#include "editor_asset.hpp"
#include <vector>

namespace ek {

class audio_asset_t : public editor_asset_t {
public:
    explicit audio_asset_t(std::string path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void load() override;

    void unload() override;

    void gui() override;

    void export_() override;

    void save() override;

    void export_meta(output_memory_stream& output) override;

private:
    std::vector<std::string> sound_list_;
    std::vector<std::string> music_list_;
};

}

