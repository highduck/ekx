#include "program_asset.hpp"

#include <scenex/asset2/asset_manager.hpp>
#include <ek/serialize/serialize.hpp>
#include <ek/system/system.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/logger.hpp>
#include <ek/assets.hpp>
#include <graphics/program.hpp>
#include <graphics/vertex_decl.hpp>

#include <pugixml.hpp>
#include <ek/editor/gui/editor_widgets.hpp>
#include <scenex/data/program_data.hpp>

namespace ek {

program_asset_t::program_asset_t(path_t path)
        : editor_asset_t{std::move(path), "program"} {
}

void program_asset_t::read_decl_from_xml(const pugi::xml_node& node) {
    frag_ = read_text(project->base_path / node.child("fragment").attribute("path").as_string());
    vert_ = read_text(project->base_path / node.child("vertex").attribute("path").as_string());
    vertex_decl_ = node.attribute("vertex_layout").as_string("2d");
}

void program_asset_t::load() {
    read_decl();

    auto* pr = new program_t(vert_.c_str(), frag_.c_str());
    if (vertex_decl_ == "2d") {
        pr->vertex = &vertex_2d::decl;
    } else if (vertex_decl_ == "3d") {
        pr->vertex = &vertex_3d::decl;
    }
    asset_t<program_t>{name_}.reset(pr);
}

void program_asset_t::unload() {
    asset_t<program_t>{name_}.reset(nullptr);
}

void program_asset_t::gui() {
    ImGui::LabelText("VS", "%s", vert_.c_str());
    ImGui::LabelText("FS", "%s", frag_.c_str());
    ImGui::LabelText("Layout", "%s", vertex_decl_.c_str());
}

void program_asset_t::build(assets_build_struct_t& data) {
    read_decl();

    const auto output_path = data.output / name_;
    scenex::program_data_t pr{};
    pr.fragment_shader = frag_;
    pr.vertex_shader = vert_;
    pr.vertex_layout = vertex_decl_;

    make_dirs(output_path.dir());

    output_memory_stream out{100};
    IO io{out};
    io(pr);
    ::ek::save(out, output_path.str() + ".program");

    data.meta(type_name_, name_);
}

void program_asset_t::save() {
    // TODO:
//    pugi::xml_document xml;
//    //if (xml.load_file(path_join(project->base_path, path_).c_str())) {
//    auto node = xml.append_child("asset");
//    node.append_attribute("name").set_value(name_.c_str());
//    node.append_attribute("type").set_value("flash");
//    node.append_attribute("path").set_value(flash_path_.c_str());
//    to_xml(node.append_child("atlas"), atlas_decl_);
//
//    const auto full_path = path_join(project->base_path, path_);
//    if (!xml.save_file(full_path.c_str())) {
//        log(log_level::error, "Error write xml file %s", full_path.c_str());
//    }
}


}