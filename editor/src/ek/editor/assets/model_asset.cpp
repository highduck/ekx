#include "model_asset.hpp"

#include <ek/fs/path.hpp>
#include <scenex/asset2/asset_manager.hpp>
#include <ek/system/system.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/logger.hpp>
#include <ek/assets.hpp>
#include <graphics/vertex_decl.hpp>

#include <pugixml.hpp>
#include <ek/editor/gui/editor_widgets.hpp>
#include <scenex/data/texture_data.hpp>
#include <scenex/data/model_data.hpp>
#include <scenex/3d/obj_loader.hpp>
#include <platform/static_resources.hpp>

using scenex::asset_object_t;

namespace ek {

model_asset_t::model_asset_t(std::string path)
        : path_{std::move(path)} {
}

void model_asset_t::load() {
    pugi::xml_document xml;

    const auto full_path = project_->base_path / path_;
    if (xml.load_file(full_path.c_str())) {
        auto node = xml.first_child();
        name_ = node.attribute("name").as_string();
        path_ = path_t{node.attribute("path").as_string()};
    } else {
        EK_ERROR << "Error parse xml " << full_path;
    }

    auto buffer = get_resource_content(get_relative_path(path_).c_str());
    if (buffer.empty()) {
        EK_ERROR << "Not found or empty " << full_path;
    } else {
        asset_t<scenex::static_mesh_t>{name_}.reset(
                new scenex::static_mesh_t(
                        scenex::load_obj(buffer)
                )
        );
    }
}

void model_asset_t::unload() {
    asset_t<scenex::static_mesh_t>{name_}.reset(nullptr);
}

void model_asset_t::gui() {
    if (ImGui::TreeNode(this, "%s (Texture Asset)", path_.c_str())) {
        ImGui::LabelText("Name", "%s", name_.c_str());
        gui_asset_object_controls(this);
        ImGui::TreePop();
    }
}

void model_asset_t::export_() {
    auto output_path = project_->export_path / name_;
    scenex::model_data_t data{
            scenex::load_obj(get_resource_content(get_relative_path(path_).c_str()))
    };

    make_dirs(output_path.dir());

    output_memory_stream out{100};
    IO io{out};
    io(data);
    ::ek::save(out, output_path + ".model");
}

void model_asset_t::save() {
    // TODO:
}

void model_asset_t::export_meta(output_memory_stream& output) {
    IO io{output};
    std::string type_name{"model"};
    io(type_name, name_);
}

path_t model_asset_t::get_relative_path(const path_t& path) const {
    return (project_->base_path / path_).dir() / path;
}

}