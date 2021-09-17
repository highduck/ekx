#pragma once

#include <vector>
#include <pugixml.hpp>
#include <ek/xfl/Doc.hpp>
#include <ek/scenex/data/SGFile.hpp>
#include <ek/builders/xfl/SGBuilder.hpp>
#include <ek/builders/xfl/RenderElement.hpp>
#include <ek/system/system.hpp>
#include "atlas.hpp"

namespace ek {

void exportFlash(const char* xmlPath) {
    pugi::xml_document xml;
    if (!xml.load_file(xmlPath)) {
        EK_ERROR_F("error parse xml %s", xmlPath);
        return;
    }
    auto node = xml.first_child();
    const auto* name = node.attribute("name").as_string();
    const auto* path = node.attribute("path").as_string();
    const auto* output = node.attribute("output").as_string();
    const auto* outputImages = node.attribute("outputImages").as_string();

    MultiResAtlasSettings atlasSettings{};
    atlasSettings.readFromXML(node.child("atlas"));
    MultiResAtlasData atlas{atlasSettings};

    xfl::Doc ff{path_t{path}};
    xfl::SGBuilder fe{ff};
    fe.build_library();

    fe.build_sprites(atlas);

    EK_DEBUG_F("Export Flash asset: %s", current_working_directory().c_str());
    auto sg_data = fe.export_library();
    output_memory_stream out{100};
    IO io{out};
    io(sg_data);
    ek::save(out, output);

    saveImages(atlas, outputImages);
}


///// prerender flash symbol


void destroy_sprite_data(SpriteData& spr) {
    //if (!spr.preserve_pixels) {
    if (spr.image) {
        delete spr.image;
        spr.image = nullptr;
    }
    //}
}

void save_sprite_png(const SpriteData& spr, const path_t& path, bool alpha = true) {
    saveImagePNG(*spr.image, path.str(), alpha);
}

// prerender_flash INPUT SYMBOL [Scale WIDTH HEIGHT ALPHA TRIM OUTPUT_PATH]
void runFlashFilePrerender(int argc, char** argv) {
    path_t inputPath{argv[0]};
    xfl::Doc ff{inputPath};
    xfl::SGBuilder exporter{ff};
    auto& doc = exporter.doc;

    auto* item = doc.findLinkage(argv[1]);
    if (item) {
        int i = 2;
        while (i < argc) {
            const float scale = strtof(argv[i], nullptr);
            const int width = atoi(argv[i + 1]);
            const int height = atoi(argv[i + 2]);
            const bool alpha = atoi(argv[i + 3]) != 0;
            const bool trim = atoi(argv[i + 4]) != 0;
            const path_t output{argv[i + 5]};

            xfl::RenderElementOptions opts{scale, width, height, alpha, trim};
            auto spr = renderElement(doc, *item, opts);
            save_sprite_png(spr, output, alpha);
            destroy_sprite_data(spr);
            i += 6;
        }
    }
}

}