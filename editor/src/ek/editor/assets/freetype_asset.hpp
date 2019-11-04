#pragma once

#include <scenex/asset2/asset_object.hpp>
#include <ek/spritepack/atlas_declaration.hpp>
#include <fonts/filters_declaration.hpp>
#include <fonts/font_declaration.hpp>

namespace ek {

class freetype_asset_t : public scenex::asset_object_t {
public:
    explicit freetype_asset_t(std::string path);

    void load() override;

    void unload() override;

    void gui() override;

    void export_() override;

    void save() override;

    void export_meta(output_memory_stream& output) override;

private:
    std::string path_;
    std::string name_;
    std::string face_path_;
    font_decl_t font_decl_;
    atlas_decl_t atlas_decl_;
    filters_decl_t filters_decl_;
};

}