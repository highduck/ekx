#pragma once

#include "editor_asset.hpp"
#include <ek/ds/Array.hpp>

namespace ek {

class audio_asset_t : public editor_asset_t {
public:
    inline static const char* type_name = "audio";

    explicit audio_asset_t(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void build(assets_build_struct_t& data) override;

private:

    std::string getRelativeName(const std::string& filepath) const;

    Array<std::string> sound_list_{};
    Array<std::string> music_list_{};
};

}

