#include "project_config.hpp"
#include "process_market_assets.hpp"

#include <ek/flash/doc/flash_archive.h>
#include <ek/system/system.hpp>
#include <ek/logger.hpp>
#include <ek/fs/path.hpp>

using namespace ek::flash;

using namespace ek;

namespace ekc {

//void process_flash_archive(const std::string& output, const std::string& name, const flash_file& file) {
//    flash_doc_exporter exporter{file};
//    exporter.build_library();
//    working_dir_t::with(output, [&] {
//        exporter.export_atlas(name);
//        exporter.export_library(name);
//    });
//}

static std::unique_ptr<basic_entry> load_flash_archive(const path_t& dir, const std::string& file) {
    const auto src = dir / file;

    // dir/FILE/FILE.xfl
    const auto xfl_file = src / file + ".xfl";
    if (is_dir(src) && is_file(xfl_file)) {
        EK_DEBUG << "XFL detected: " << xfl_file;
        return std::make_unique<xfl_entry>(src);
    }

    // dir/FILE.fla
    const auto fla_file = src + ".fla";
    if (is_file(fla_file)) {
        EK_DEBUG << "FLA detected: " << fla_file;
        return std::make_unique<fla_entry>(fla_file);
    }

    return nullptr;
}

void process_market_asset(const project_config_t& config) {
    using namespace ek::flash;
    using ek::path_join;

    auto arch = load_flash_archive(config.marketing.input, config.marketing.name);
    if (arch) {
        flash_file ff{std::move(arch)};
        process_flash_archive_market(ff);
    } else {
        EK_ERROR << "process_market_asset " << config.marketing.input << ' ' << config.marketing.name;
    }
}

}