#include "export_marketing.hpp"

#include <ek/flash/rasterizer/render_to_sprite.hpp>
#include <ek/flash/doc/flash_archive.hpp>
#include <ek/spritepack/save_image_png.hpp>
#include <ek/xfl/flash_doc_exporter.hpp>
#include <ek/editor/assets/flash_asset.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/util/strings.hpp>
#include <ek/util/logger.hpp>

#include <nlohmann/json.hpp>
#include <fmt/core.h>
#include <iomanip>
#include <fstream>

using json = nlohmann::json;

using namespace ek::flash;
using namespace ek::spritepack;
using namespace std;

namespace ek {

void destroy_sprite_data(sprite_t& spr) {
    //if (!spr.preserve_pixels) {
    if (spr.image) {
        delete spr.image;
        spr.image = nullptr;
    }
    //}
}

void save_sprite_png(const sprite_t& spr, const path_t& path, bool alpha = true) {
    save_image_png_stb(*spr.image, path.str(), alpha);
}

void store_hi_res_icon(const flash_doc& doc, const element_t& symbol, int size, const std::string& prefix = "icon_") {
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

void render_ios_icons(const flash_doc& doc, const element_t& symbol) {
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
        // disable alpha: AppStore require OPAQUE icons
        save_sprite_png(spr, path / filename, false);
        destroy_sprite_data(spr);

        image["filename"] = filename;
        images.emplace_back(image);
    }

    {
        ofstream os{(path / "Contents.json").str()};
        os << setw(4) << json{{"images", images}} << endl;
    }
}

void render_android_icons(const flash_doc& doc, const element_t& symbol, const string& name) {
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

void process_flash_archive_market(const flash_doc& file, const marketing_asset_t& marketing) {
    flash_doc_exporter exporter{file};
    auto& doc = exporter.doc;

    for (const auto& command_data : marketing.commands) {
        make_dirs(command_data.output);
        working_dir_t wd;
        const auto* icon_item = exporter.doc.find_linkage("icon");
        const auto* icon_round_item = exporter.doc.find_linkage("icon_round");
        wd.in(command_data.output, [&]() {
            if (command_data.target == "gen") {
                if (icon_item) {
                    for (auto size : {16, 32, 256, 512, 1024}) {
                        store_hi_res_icon(doc, *icon_item, size);
                    }
                }
                auto* item = exporter.doc.find_linkage("feature_graphic");
                if (item) {
                    renderer_options_t opts{1.0f, 1024, 500, false, true};
                    auto spr = render(doc, *item, opts);
                    save_sprite_png(spr, path_t{"_feature_graphic.png"});
                    destroy_sprite_data(spr);
                }
                item = exporter.doc.find_linkage("promo_graphic");
                if (item) {
                    renderer_options_t opts{1.0f, 180, 120, false, true};
                    auto spr = render(doc, *item, opts);
                    save_sprite_png(spr, path_t{"_promo_graphic.png"});
                    destroy_sprite_data(spr);
                }
                item = exporter.doc.find_linkage("btn_app_store");
                if (item) {
                    renderer_options_t opts{2.0f, 0, 0, false, false};
                    auto spr = render(doc, *item, opts);
                    save_sprite_png(spr, path_t{"btn_app_store.png"});
                    destroy_sprite_data(spr);
                }
                item = exporter.doc.find_linkage("btn_google_play");
                if (item) {
                    renderer_options_t opts{2.0f, 0, 0, false, false};
                    auto spr = render(doc, *item, opts);
                    save_sprite_png(spr, path_t{"btn_google_play.png"});
                    destroy_sprite_data(spr);
                }
                for (const auto& s: exporter.doc.library) {
                    if (starts_with(s.item.name, "achievements/") || starts_with(s.item.name, "leaderboards/")) {
                        auto dir = path_t{s.item.name}.dir();
                        make_dirs(dir);
                        wd.in(dir, [&]() {
                            renderer_options_t opts{1.0f, 512, 512, false, false};
                            auto spr = render(doc, s, opts);
                            save_sprite_png(spr, path_t{s.timeline.name + ".png"});
                            destroy_sprite_data(spr);
                        });
                    }
                }
            } else if (command_data.target == "android") {
                if (icon_item) {
                    render_android_icons(doc, *icon_item, "ic_launcher.png");
                }
            } else if (command_data.target == "ios") {
                if (icon_item) {
                    render_ios_icons(doc, *icon_item);
                }
            } else if (command_data.target == "web") {
                if (icon_item) {
                    for (auto size : {36, 48, 72, 96, 144, 192, 256, 512}) {
                        store_hi_res_icon(doc, *icon_item, size, "icon");
                    }
                }
            }
        });
    }
}

void process_market_asset(const marketing_asset_t& marketing) {
    using namespace ek::flash;
    using ek::path_join;
    
    flash_doc ff{marketing.input};
    process_flash_archive_market(ff, marketing);
}

}