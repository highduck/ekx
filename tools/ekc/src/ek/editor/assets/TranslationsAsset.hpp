#pragma once

#include "editor_asset.hpp"
#include <ek/ds/Array.hpp>

namespace ek {

class TranslationsAsset : public editor_asset_t {
public:
    inline static const char* type_name = "translations";

    explicit TranslationsAsset(path_t path);

    void read_decl_from_xml(const pugi::xml_node& node) override;

    void build(assets_build_struct_t& data) override;

private:

    std::unordered_map<std::string, path_t> languages;
    std::unordered_map<std::string, Array<uint8_t>> convertedData;
};

}

