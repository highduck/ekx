#include "flash_asset.hpp"

#include <memory>
#include <utility>
#include <ek/debug.hpp>
#include <ek/util/Res.hpp>
#include <ek/xfl/Doc.hpp>
#include <ek/scenex/data/SGFile.hpp>
#include <ek/builders/xfl/SGBuilder.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>

namespace ek {

flash_asset_t::flash_asset_t(path_t path) :
        editor_asset_t{std::move(path), "flash"} {
}

void flash_asset_t::read_decl_from_xml(const pugi::xml_node& node) {
    atlasTarget_ = node.attribute("atlas").value();
}

void flash_asset_t::build(assets_build_struct_t& data) {
    read_decl();

    xfl::Doc ff{project->base_path / resource_path_};
    xfl::SGBuilder fe{ff};
    fe.build_library();

    Res<MultiResAtlasData> atlasBuild{atlasTarget_};
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

}