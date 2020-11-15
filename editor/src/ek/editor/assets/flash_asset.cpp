#include "flash_asset.hpp"

#include <ek/util/logger.hpp>
#include <utility>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/flash/doc/flash_doc.hpp>
#include <ek/spritepack/export_atlas.hpp>
#include <ek/scenex/2d/atlas.hpp>
#include <ek/scenex/2d/sprite.hpp>
#include <ek/scenex/data/sg_data.hpp>
#include <ek/xfl/flash_doc_exporter.hpp>
#include <ek/flash/doc/flash_archive.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <memory>
#include <ek/editor/gui/gui.hpp>

namespace ek {

flash_asset_t::flash_asset_t(path_t path)
        : editor_asset_t{std::move(path), "flash"} {
    reloadOnScaleFactorChanged = true;
}

void flash_asset_t::read_decl_from_xml(const pugi::xml_node& node) {
    atlasTarget_ = node.attribute("atlas").value();
}

void flash_asset_t::load() {
    read_decl();

    flash::flash_doc ff{project->base_path / resource_path_};
    flash::flash_doc_exporter fe{ff};
    fe.build_library();

    asset_t<spritepack::atlas_t> atlasBuild{atlasTarget_};
    fe.build_sprites(atlasBuild.mutableRef());

    sg_file sg = fe.export_library();
    asset_t<sg_file>{name_}.reset(new sg_file{sg});
}

void flash_asset_t::unload() {
    asset_t<sg_file>{name_}.reset(nullptr);
}

void flash_asset_t::gui() {
    asset_t<sg_file> library{name_};
    gui_sg_file_view(library.get());
}

void flash_asset_t::build(assets_build_struct_t& data) {
    read_decl();

    flash::flash_doc ff{project->base_path / resource_path_};
    flash::flash_doc_exporter fe{ff};
    fe.build_library();

    asset_t<spritepack::atlas_t> atlasBuild{atlasTarget_};
    fe.build_sprites(atlasBuild.mutableRef());

    make_dirs(data.output);
    working_dir_t::with(data.output, [&] {
        EK_DEBUG << "Export Flash asset: " << current_working_directory();
        auto sg_data = fe.export_library();
        output_memory_stream out{100};
        IO io{out};
        io(sg_data);
        ek::save(out, name_ + ".sg");
    });

    data.meta("scene", name_);
}

void flash_asset_t::save() {
    pugi::xml_document xml;
    //if (xml.load_file(path_join(project->base_path, path_).c_str())) {
    auto node = xml.append_child("asset");
    node.append_attribute("name").set_value(name_.c_str());
    node.append_attribute("type").set_value("flash");
    node.append_attribute("path").set_value(resource_path_.c_str());
    node.append_attribute("atlas").set_value(atlasTarget_.c_str());

    const auto full_path = project->base_path / declaration_path_;
    if (!xml.save_file(full_path.c_str())) {
        EK_ERROR << "Error write xml file " << full_path;
    }
}

}