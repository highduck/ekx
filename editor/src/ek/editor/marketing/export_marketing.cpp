#include "export_marketing.hpp"

#include <ek/logger.hpp>
#include <ek/flash/rasterizer/render_to_sprite.h>
#include <ek/flash/doc/flash_archive.h>
#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/utility/strings.hpp>
#include <xfl/flash_doc_exporter.h>
#include <ek/spritepack/save_image_png.hpp>

#include <nlohmann/json.hpp>
#include <fmt/core.h>
#include <iomanip>
#include <fstream>

using json = nlohmann::json;

using namespace ek::flash;
using namespace ek::spritepack;
using namespace std;

namespace ek {

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

void destroy_sprite_data(sprite_t& spr) {
    //if (!spr.preserve_pixels) {
    if (spr.image) {
        delete spr.image;
        spr.image = nullptr;
    }
    //}
}

void save_sprite_png(const sprite_t& spr, const path_t& path) {
    save_image_png_stb(*spr.image, path.str());
}

void store_hi_res_icon(const flash_file& doc, const element_t& symbol, int size, const std::string& prefix = "icon_") {
    renderer_options_t opts{float(size) / 64.0f, size, size, true, true};
    auto spr = render(doc, symbol, opts);
    path_t icon_path{prefix + std::to_string(size) + ".png"};
    save_sprite_png(spr, icon_path);
    destroy_sprite_data(spr);
}

json create_ios_icons_template() {
    const auto path = path_t{getenv("EKX_ROOT")}
                      / "ek/templates/template-ios/src/Assets.xcassets/AppIcon.appiconset/Contents.json";
    json res;
    ifstream is(path.c_str());
    is >> res;
    return res;
}

void render_ios_icons(const flash_file& doc, const element_t& symbol) {
    const path_t path{"AppIcon.appiconset"};
    make_dir(path);
    json ios_icon = create_ios_icons_template();
    json images = json::array();
    const float original_size = 64.0f;
    for (auto image : ios_icon["images"]) {
        const float scale_factor = strtof(image["scale"].get<string>().c_str(), nullptr);
        const float size = scale_factor * strtof(image["size"].get<string>().c_str(), nullptr);
        const int sizei = static_cast<int>(size);
        string filename = fmt::format("{}_{}.png", image["idiom"].get<string>(), sizei);

        renderer_options_t opts{size / original_size,
                                sizei, sizei,
                                true, true};
        auto spr = render(doc, symbol, opts);
        save_sprite_png(spr, path / filename);
        destroy_sprite_data(spr);

        image["filename"] = filename;
        images.emplace_back(image);
    }

    {
        ofstream os{(path / "Contents.json").str()};
        os << setw(4) << json{{"images", images}} << endl;
    }
}

void render_android_icons(const flash_file& doc, const element_t& symbol, const string& name) {
    map<string, int> resolution_map{
            {"ldpi",    36},
            {"mdpi",    48},
            {"hdpi",    72},
            {"xhdpi",   96},
            {"xxhdpi",  144},
            {"xxxhdpi", 192},
    };
    float original_size = 64.0f;
    float scale_factor = 1.0f / original_size;
    for (const auto& resolution: resolution_map) {
        auto size = resolution.second;
        renderer_options_t opts{scale_factor * size,
                                size, size,
                                true, true};
        path_t dir{"mipmap-" + resolution.first};
        make_dir(dir);

        auto spr = render(doc, symbol, opts);
        save_sprite_png(spr, dir / name);
        destroy_sprite_data(spr);
    }
}

void process_flash_archive_market(const ek::flash::flash_file& file) {
    flash_doc_exporter exporter{file};
    auto& doc = exporter.doc;

    const string marketing_folder = "generated/marketing";
    const string marketing_achievements_folder = "generated/marketing/achievements";
    const string marketing_leaderboards_folder = "generated/marketing/leaderboards";
    const string android_res_folder = "generated/android/res";
    const string ios_res_folder = "generated/ios";
    const string pwa_icons_folder = "generated/pwa/icons";

    make_dirs(marketing_folder);
    make_dirs(marketing_achievements_folder);
    make_dirs(marketing_leaderboards_folder);
    make_dirs(android_res_folder);
    make_dirs(ios_res_folder);
    make_dirs(pwa_icons_folder);
    for (const auto& item: exporter.doc.library) {
        if (item.item.linkageClassName == "icon") {

            working_dir_t::with(marketing_folder, [&]() {
                for (auto size : {16, 32, 256, 512, 1024}) {
                    store_hi_res_icon(doc, item, size);
                }
            });

            working_dir_t::with(ios_res_folder, [&]() {
                render_ios_icons(doc, item);
            });
            working_dir_t::with(android_res_folder, [&]() {
                render_android_icons(doc, item, "ic_launcher.png");
            });

            working_dir_t::with(pwa_icons_folder, [&]() {
                for (auto size : {36, 48, 72, 96, 144, 192, 256, 512}) {
                    store_hi_res_icon(doc, item, size, "icon");
                }
            });

        } else if (item.item.linkageClassName == "icon_round") {
            working_dir_t::with(android_res_folder, [&]() {
                render_android_icons(doc, item, "ic_launcher_round.png");
            });
        } else if (item.item.linkageClassName == "feature_graphic") {
            working_dir_t::with(marketing_folder, [&]() {
                renderer_options_t opts{1.0f, 1024, 500, false, true};
                auto spr = render(doc, item, opts);
                save_sprite_png(spr, path_t{"_feature_graphic.png"});
                destroy_sprite_data(spr);
            });
        } else if (item.item.linkageClassName == "promo_graphic") {
            working_dir_t::with(marketing_folder, [&]() {
                renderer_options_t opts{1.0f, 180, 120, false, true};
                auto spr = render(doc, item, opts);
                save_sprite_png(spr, path_t{"_promo_graphic.png"});
                destroy_sprite_data(spr);
            });
        } else if (item.item.linkageClassName == "btn_app_store"
                   || item.item.linkageClassName == "btn_google_play") {
            working_dir_t::with(marketing_folder, [&]() {
                renderer_options_t opts{2.0f, 0, 0, false, false};
                auto spr = render(doc, item, opts);
                save_sprite_png(spr, path_t{item.item.linkageClassName + ".png"});
                destroy_sprite_data(spr);
            });
        } else if (ek::starts_with(item.item.name, "achievements/")) {
            working_dir_t::with(marketing_achievements_folder, [&]() {
                renderer_options_t opts{1.0f, 512, 512, false, false};
                auto spr = render(doc, item, opts);
                save_sprite_png(spr, path_t{item.timeline.name + ".png"});
                destroy_sprite_data(spr);
            });
        } else if (ek::starts_with(item.item.name, "leaderboards/")) {
            working_dir_t::with(marketing_leaderboards_folder, [&]() {
                renderer_options_t opts{1.0f, 512, 512, false, false};
                auto spr = render(doc, item, opts);
                save_sprite_png(spr, path_t{item.timeline.name + ".png"});
                destroy_sprite_data(spr);
            });
        }
    }
}

void process_market_asset(const marketing_asset_t& marketing) {
    using namespace ek::flash;
    using ek::path_join;

    auto arch = load_flash_archive(marketing.input, marketing.name);
    if (arch) {
        flash_file ff{std::move(arch)};
        process_flash_archive_market(ff);
    } else {
        EK_ERROR << "process_market_asset " << marketing.input << ' ' << marketing.name;
    }
}

}