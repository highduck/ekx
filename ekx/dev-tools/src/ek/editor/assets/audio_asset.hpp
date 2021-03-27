#pragma once

#include "editor_asset.hpp"
#include <vector>

namespace ek {

class audio_asset_t : public editor_asset_t {
public:
    inline static const char* type_name = "audio";

    explicit audio_asset_t(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void load() override;

    void unload() override;

    void gui() override;

    void build(assets_build_struct_t& data) override;

    void save() override;

private:

    std::string getRelativeName(const std::string& filepath) const;

    std::vector<std::string> sound_list_;
    std::vector<std::string> music_list_;
};

}

