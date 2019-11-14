#include "flash_asset.hpp"

#include <scenex/asset2/asset_manager.hpp>
#include <ek/logger.hpp>
#include <utility>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/flash/doc/flash_file.h>
#include <ek/spritepack/export_atlas.hpp>
#include <scenex/2d/atlas.hpp>
#include <scenex/2d/sprite.hpp>
#include <scenex/data/sg_data.h>
#include <xfl/flash_doc_exporter.h>
#include <ek/flash/doc/flash_archive.h>
#include <ek/editor/gui/editor_widgets.hpp>
#include <ek/editor/assets/editor_temp_atlas.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <memory>

using scenex::asset_object_t;

namespace ek {

std::unique_ptr<flash::basic_entry> load_flash_archive(const path_t& path) {
    using namespace ek::flash;

    if (is_file(path)) {
        const auto ext = path.ext();
        // dir/FILE/FILE.xfl
        if (ext == "xfl") {
            auto dir = path.dir();
            if (is_dir(dir)) {
                return std::make_unique<xfl_entry>(dir);
            } else {
                EK_ERROR("Import Flash: loading %s XFL file, but %s is not a dir", path.c_str(), dir.c_str());
            }
        } else if (ext == "fla") {
            return std::make_unique<fla_entry>(path);
        }
        else {
            EK_ERROR << "Import Flash: file is not xfl or fla: " << path;
        }
    }

    // dir/FILE.fla
    const auto fla_file = path + ".fla";
    if (is_file(fla_file)) {
        return std::make_unique<fla_entry>(fla_file);
    } else if (is_dir(path)) {
        if (is_file(path / path.basename() + ".xfl")) {
            return std::make_unique<xfl_entry>(path);
        } else {
            EK_WARN << "Import Flash: given dir doesn't contain .xfl file: " << path;
        }
    }

    EK_ERROR << "Import Flash: file not found: " << path;

    return nullptr;
}


flash_asset_t::flash_asset_t(std::string path)
        : path_{std::move(path)} {
}

void flash_asset_t::load() {
    pugi::xml_document xml;
    const auto full_path = (project_->base_path / path_);
    if (xml.load_file(full_path.c_str())) {
        auto node = xml.first_child();
        name_ = node.attribute("name").as_string();
        flash_path_ = node.attribute("path").as_string();

        atlas_decl_ = {};
        from_xml(node.child("atlas"), atlas_decl_);
        if (atlas_decl_.name.empty()) {
            atlas_decl_.name = name_;
        }
    } else {
        EK_ERROR << "Error parse xml " << full_path;
    }

    flash::flash_file ff{load_flash_archive(project_->base_path / flash_path_)};
    flash::flash_doc_exporter fe{ff};
    fe.build_library();

    auto temp_atlas = prepare_temp_atlas(name_, project_->scale_factor);
    fe.build_sprites(temp_atlas);
    load_temp_atlas(temp_atlas);

    auto* sg = new scenex::sg_file;
    sg->library = fe.library.node;
    sg->linkages = fe.linkages;
    asset_t<scenex::sg_file>{name_}.reset(sg);
}

void flash_asset_t::unload() {
    asset_t<scenex::atlas_t>{name_}.reset(nullptr);
    asset_t<scenex::sg_file>{name_}.reset(nullptr);
}

void flash_asset_t::gui() {
    if (ImGui::TreeNode(this, "%s (Flash)", path_.c_str())) {
        ImGui::LabelText("Name", "%s", name_.c_str());
        ImGui::LabelText("Flash File", "%s", flash_path_.c_str());

        gui_asset_object_controls(this);

        asset_t<scenex::atlas_t> atlas{name_};
        gui_atlas_view(atlas.get());

        asset_t<scenex::sg_file> library{name_};
        gui_sg_file_view(library.get());

        ImGui::TreePop();
    }
}

void flash_asset_t::export_() {
    flash::flash_file ff{load_flash_archive(project_->base_path / flash_path_)};
    flash::flash_doc_exporter fe{ff};
    fe.build_library();

    spritepack::atlas_t temp_atlas{atlas_decl_};
    fe.build_sprites(temp_atlas);

    make_dirs(path_t{project_->export_path});
    working_dir_t::with(project_->export_path, [&] {
        EK_DEBUG << "Export Flash asset: " << current_working_directory();
        spritepack::export_atlas(temp_atlas);
        auto sg_data = fe.export_library();
        output_memory_stream out{100};
        IO io{out};
        io(sg_data);
        ek::save(out, name_ + ".sg");
    });
}

void flash_asset_t::save() {
    pugi::xml_document xml;
    //if (xml.load_file(path_join(project_->base_path, path_).c_str())) {
    auto node = xml.append_child("asset");
    node.append_attribute("name").set_value(name_.c_str());
    node.append_attribute("type").set_value("flash");
    node.append_attribute("path").set_value(flash_path_.c_str());
    to_xml(node.append_child("atlas"), atlas_decl_);

    const auto full_path = project_->base_path / path_;
    if (!xml.save_file(full_path.c_str())) {
        EK_ERROR << "Error write xml file " << full_path;
    }
}

void flash_asset_t::export_meta(output_memory_stream& output) {
    IO io{output};
    std::string atlas_type{"atlas"};
    std::string scene_type{"scene"};
    io(atlas_type, name_);
    io(scene_type, name_);
}

}