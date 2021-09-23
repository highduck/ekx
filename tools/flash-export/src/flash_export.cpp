#include <vector>
#include <pugixml.hpp>
#include "xfl/Doc.hpp"
#include <ek/scenex/data/SGFile.hpp>
#include <ek/debug/LogSystem.hpp>
#include "sxg/SGBuilder.hpp"
#include "sxg/RenderElement.hpp"
#include "MultiResAtlas.hpp"
//#include <ek/system/system.hpp>

#include <stb/stb_image_write.h>

namespace ek {

void undoPremultiplyAlpha(image_t& bitmap) {
    auto* it = (abgr32_t*) bitmap.data();
    const auto* end = it + bitmap.width() * bitmap.height();

    while (it < end) {
        const auto alpha = it->a;
        if (alpha == 0) {
            *(uint32_t*) it = 0u;
        } else if (alpha < 0xFFu) {
            it->r = static_cast<uint8_t>((0xFFu * it->r) / alpha);
            it->g = static_cast<uint8_t>((0xFFu * it->g) / alpha);
            it->b = static_cast<uint8_t>((0xFFu * it->b) / alpha);
        }
        ++it;
    }
}

void save(MultiResAtlasData& images, const char* output) {
    pugi::xml_document doc{};
    auto nodeAtlas = doc.append_child("images");
    int idx = 0;
    char path[1024];

    for (auto& resolution: images.resolutions) {
        auto nodeResolution = nodeAtlas.append_child("resolution");
        for (auto& image: resolution.sprites) {
            if (image.image) {
                auto& bitmap = *image.image;
                auto nodeSprite = nodeResolution.append_child("image");
                snprintf(path, 1024, "%s/%d.bmp", output, idx++);
                // require RGBA non-premultiplied alpha
                undoPremultiplyAlpha(bitmap);
                stbi_write_bmp(path, (int)bitmap.width(), (int)bitmap.height(), 4, bitmap.data());

                nodeSprite.append_attribute("path").set_value(path);
                nodeSprite.append_attribute("name").set_value(image.name.c_str());
                nodeSprite.append_attribute("x").set_value(image.rc.x);
                nodeSprite.append_attribute("y").set_value(image.rc.y);
                nodeSprite.append_attribute("w").set_value(image.rc.width);
                nodeSprite.append_attribute("h").set_value(image.rc.height);
                nodeSprite.append_attribute("p").set_value(image.padding);
            }
        }
    }
    snprintf(path, 1024, "%s/_images.xml", output);
    doc.save_file(path);
}

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

    auto sg_data = fe.export_library();
    output_memory_stream out{100};
    IO io{out};
    io(sg_data);

    auto f = fopen(output, "wb");
    fwrite(out.data(), out.size(), 1, f);
    fclose(f);

    save(atlas, outputImages);
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
    using namespace ek;

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


int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    if (strcmp("render", argv[1]) == 0) {
        ek::runFlashFilePrerender(argc - 2, argv + 2);
    } else if (strcmp("export", argv[1]) == 0) {
        if (argc > 2) {
            ek::exportFlash(argv[2]);
        } else {
            return 1;
        }
    }
    return 0;

}