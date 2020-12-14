#include "export_marketing.hpp"

#include <ek/flash/rasterizer/render_to_sprite.hpp>
#include <ek/flash/doc/flash_archive.hpp>
#include <ek/spritepack/save_image_png.hpp>
#include <ek/xfl/flash_doc_exporter.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/util/strings.hpp>

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

void process_flash_archive_market(const flash_doc& file, const marketing_asset_t& marketing) {
    flash_doc_exporter exporter{file};
    auto& doc = exporter.doc;

    for (const auto& command_data : marketing.commands) {
        make_dirs(command_data.output);
        working_dir_t wd;
        const auto* icon_item = exporter.doc.find_linkage("icon_market");
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

// prerender_flash INPUT SYMBOL [Scale WIDTH HEIGHT ALPHA TRIM OUTPUT_PATH]
void runFlashFilePrerender(const vector<std::string>& args) {
    path_t inputPath{args[2]};
    flash_doc ff{inputPath};
    flash_doc_exporter exporter{ff};
    auto& doc = exporter.doc;

    auto* item = doc.find_linkage(args[3]);
    if (item) {
        int i = 4;
        while (i < args.size()) {
            const float scale = strtof(args[i].c_str(), nullptr);
            const int width = atoi(args[i + 1].c_str());
            const int height = atoi(args[i + 2].c_str());
            const bool alpha = atoi(args[i + 3].c_str()) != 0;
            const bool trim = atoi(args[i + 4].c_str()) != 0;
            const path_t output{args[i + 5]};

            renderer_options_t opts{scale, width, height, alpha, trim};
            auto spr = render(doc, *item, opts);
            save_sprite_png(spr, output, alpha);
            destroy_sprite_data(spr);
            i += 6;
        }
    }
}

}