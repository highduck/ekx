#include "freetype_asset.hpp"

#include <ek/editor/gui/editor_widgets.hpp>
#include <scenex/asset2/asset_manager.hpp>
#include <ek/editor/assets/editor_temp_atlas.hpp>
#include <ek/logger.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/spritepack/export_atlas.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <scenex/2d/atlas.hpp>
#include <scenex/2d/sprite.hpp>
#include <scenex/data/sg_data.h>
#include <ek/system/system.hpp>
#include <fonts/export_font.h>
#include <utility>
#include <memory>

using scenex::asset_object_t;

namespace ek {

freetype_asset_t::freetype_asset_t(std::string path)
        : path_{std::move(path)} {
}

void freetype_asset_t::load() {
    pugi::xml_document xml;
    const auto full_path = project_->base_path / path_;
    if (xml.load_file(full_path.c_str())) {
        auto node = xml.first_child();
        name_ = node.attribute("name").as_string();
        face_path_ = node.attribute("path").as_string();

        atlas_decl_ = {};
        from_xml(node.child("atlas"), atlas_decl_);
        if (atlas_decl_.name.empty()) {
            atlas_decl_.name = name_;
        }

        font_decl_ = {};
        from_xml(node.child("font"), font_decl_);

        filters_decl_ = {};
        from_xml(node.child("filters"), filters_decl_);
    } else {
        EK_ERROR << "Error parse xml: " << full_path;
    }

    auto temp_atlas = prepare_temp_atlas(name_, project_->scale_factor);
    auto font_data = font_lib::export_font(project_->base_path / face_path_,
                                           name_,
                                           font_decl_,
                                           filters_decl_,
                                           temp_atlas);

    load_temp_atlas(temp_atlas);

    asset_t<scenex::font_t>{name_}.reset(new scenex::font_t(font_data));
}

void freetype_asset_t::unload() {
    asset_t<scenex::atlas_t>{name_}.reset(nullptr);
    asset_t<scenex::font_t>{name_}.reset(nullptr);
}

void freetype_asset_t::gui() {
    if (ImGui::TreeNode(this, "%s (FreeType)", path_.c_str())) {
        ImGui::LabelText("Name", "%s", name_.c_str());
        ImGui::LabelText("Face Path", "%s", face_path_.c_str());
        gui_asset_object_controls(this);

        asset_t<scenex::atlas_t> atlas{name_};
        gui_atlas_view(atlas.get());

        asset_t<scenex::font_t> font{name_};
        gui_font_view(font.get());

        ImGui::TreePop();
    }
}

void freetype_asset_t::export_() {
    spritepack::atlas_t atlas{atlas_decl_};
    auto font_data = font_lib::export_font(project_->base_path / face_path_,
                                           name_,
                                           font_decl_,
                                           filters_decl_,
                                           atlas);
    working_dir_t::with(project_->export_path, [&] {
        EK_DEBUG << "Export Freetype asset: " << current_working_directory();
        ek::output_memory_stream out{100};
        IO io{out};
        io(font_data);
        ::ek::save(out, name_ + ".font");
        spritepack::export_atlas(atlas);
    });
}

void freetype_asset_t::save() {
    // TODO:
//    pugi::xml_document xml;
//    //if (xml.load_file(path_join(project_->base_path, path_).c_str())) {
//    auto node = xml.append_child("asset");
//    node.append_attribute("name").set_value(name_.c_str());
//    node.append_attribute("type").set_value("flash");
//    node.append_attribute("path").set_value(flash_path_.c_str());
//    to_xml(node.append_child("atlas"), atlas_decl_);
//
//    const auto full_path = path_join(project_->base_path, path_);
//    if (!xml.save_file(full_path.c_str())) {
//        log(log_level::error, "Error write xml file %s", full_path.c_str());
//    }
}

void freetype_asset_t::export_meta(output_memory_stream& output) {
    IO io{output};
    std::string atlas_type{"atlas"};
    std::string font_type{"font"};
    io(atlas_type, name_);
    io(font_type, name_);
}

}