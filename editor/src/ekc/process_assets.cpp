#include "project_config.hpp"

#include <ek/system/system.hpp>

using namespace ek;
using namespace ek::font_lib;
using namespace ek::flash;

namespace ekc {

void process_market_asset(const asset_entry_config_t& asset, const std::string& output);

void process_assets(const project_config_t& config) {
    const auto& assets = config.assets;
    const auto& output = config.assets.output;

    make_dir(output);
    for (const auto& asset : assets.list) {
        if (asset.type == "market") {
            process_market_asset(asset, output.str());
        }
    }
}

}