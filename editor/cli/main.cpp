#include <platform/boot.h>
#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/logger.hpp>
#include <ek/utility/strings.hpp>

#include <ek/editor/marketing/export_marketing.hpp>

#include <ek/editor/assets/flash_asset.hpp>
#include <ek/editor/assets/audio_asset.hpp>
#include <ek/editor/assets/freetype_asset.hpp>
#include <ek/editor/assets/program_asset.hpp>
#include <ek/editor/assets/texture_asset.hpp>
#include <ek/editor/assets/model_asset.hpp>

#include <ek/editor/assets/editor_assets.hpp>

namespace ek {

// Marketing Resources generation:
// 1) Export command
// - what to export
// - platform
// - file/dir
// - type
// - output
// `ekc export market assets/res web generated/web`
void main() {
    using namespace std;
    using namespace scenex;

    EK_INFO << "== EKC utility ==";
    EK_INFO << "Executable path: " << get_executable_path();
    EK_INFO << "Working dir: " << current_working_directory();
    EK_INFO << "Arguments: ";
    auto args = get_program_arguments();
    EK_INFO << join(args, " ");

    if (args.size() <= 1) {
        return;
    }

    if (args.size() > 1) {
        if (args[1] == "export") {
            if (args.size() > 2) {
                auto what = args[2];
                if (what == "market") {
                    marketing_asset_t marketing_data{};
                    marketing_data.input = path_t{args[3]};
                    marketing_data.commands.push_back({
                                                              args[4],
                                                              path_t{args[5]}
                                                      });
                    process_market_asset(marketing_data);
                } else if (what == "assets") {
                    asset_manager_t assets{};

                    assets.base_path = path_t{args[3]};
                    assets.export_path = path_t{args[4]};

                    assets.add_resolver(new editor_asset_resolver_t<texture_asset_t>("texture"));
                    assets.add_resolver(new editor_asset_resolver_t<program_asset_t>("program"));
                    assets.add_resolver(new editor_asset_resolver_t<freetype_asset_t>("freetype"));
                    assets.add_resolver(new editor_asset_resolver_t<flash_asset_t>("flash"));
                    assets.add_resolver(new editor_asset_resolver_t<model_asset_t>("model"));
                    assets.add_resolver(new editor_asset_resolver_t<audio_asset_t>("audio"));
                    scan_assets_folder(assets, false);
                    export_all_assets(assets);
                }
            }
        }
    }


    const string marketing_folder = "generated/marketing";
    const string marketing_achievements_folder = "generated/marketing/achievements";
    const string marketing_leaderboards_folder = "generated/marketing/leaderboards";
    const string android_res_folder = "generated/android/res";
    const string ios_res_folder = "generated/ios";
    const string pwa_icons_folder = "generated/pwa/icons";
}

}

void ek_main() {

}