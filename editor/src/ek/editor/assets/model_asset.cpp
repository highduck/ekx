#include "model_asset.hpp"

#include <scenex/3d/static_mesh.hpp>
#include <ek/system/system.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/logger.hpp>
#include <ek/assets.hpp>
#include <graphics/vertex_decl.hpp>
#include <ek/editor/gui/editor_widgets.hpp>
#include <scenex/data/model_data.hpp>
#include <ek/editor/obj/obj_loader.hpp>

namespace ek {

model_asset_t::model_asset_t(std::string path)
        : editor_asset_t{std::move(path), "model"} {
}

void model_asset_t::read_decl_from_xml(const pugi::xml_node& node) {

}

void model_asset_t::load() {
    read_decl();

    auto buffer = read_file(get_relative_path(resource_path_));
    if (buffer.empty()) {
        EK_ERROR << "Not found or empty " << (project_->base_path / path_);
    } else {
        asset_t<scenex::static_mesh_t>{name_}.reset(
                new scenex::static_mesh_t(
                        load_obj(buffer)
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
    read_decl();
    
    auto output_path = project_->export_path / name_;
    scenex::model_data_t data{
            load_obj(read_file(get_relative_path(resource_path_)))
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

}