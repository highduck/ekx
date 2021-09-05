#pragma once

#include <ek/system/system.hpp>
#include <ek/builders/MultiResAtlas.hpp>
#include <ek/imaging/drawing.hpp>
#include <stb/stb_image_write.h>

#include <stb/stb_image.h>

namespace ek {

void saveImages(MultiResAtlasData& atlas, const char* output) {
    const path_t outputPath{output};
    pugi::xml_document doc{};
    auto nodeAtlas = doc.append_child("atlas");
    for (auto& resolution : atlas.resolutions) {
        auto nodeResolution = nodeAtlas.append_child("resolution");
        const path_t dir = outputPath / std::to_string(resolution.resolution_index);
        for (auto& sprite : resolution.sprites) {
            auto nodeSprite = nodeResolution.append_child("sprite");
            if (sprite.image) {
                // require RGBA non-premultiplied alpha
                undo_premultiply_image(*sprite.image);
                const auto* data = sprite.image->data();
                const int w = (int) sprite.image->width();
                const int h = (int) sprite.image->height();
                const auto nn = dir / sprite.name + ".bmp";
                make_dirs(nn.dir());
                stbi_write_bmp(nn.c_str(), w, h, 4, data);

                nodeSprite.append_attribute("path").set_value(nn.c_str());
                nodeSprite.append_attribute("name").set_value(sprite.name.c_str());
                nodeSprite.append_attribute("x").set_value(sprite.rc.x);
                nodeSprite.append_attribute("y").set_value(sprite.rc.y);
                nodeSprite.append_attribute("w").set_value(sprite.rc.width);
                nodeSprite.append_attribute("h").set_value(sprite.rc.height);
                nodeSprite.append_attribute("pad").set_value(sprite.padding);
            }
        }
    }
    doc.save_file((outputPath / "sprites.xml").c_str());
}

void exportAtlas(const char* xmlPath) {
    pugi::xml_document xml;
    if (!xml.load_file(xmlPath)) {
        EK_ERROR("error parse xml %s", xmlPath);
        return;
    }
    auto node = xml.first_child();
    const auto* output = node.attribute("output").as_string();
    MultiResAtlasSettings atlasSettings{};
    atlasSettings.readFromXML(node);
    MultiResAtlasData atlas{atlasSettings};

    for (auto& nodeInput: node.children("input")) {
        pugi::xml_document inputDoc;
        if (inputDoc.load_file(nodeInput.attribute("path").as_string())) {
            int resIndex = 0;
            for (auto& nodeInputRes : inputDoc.first_child().children("resolution")) {
                auto& res = atlas.resolutions[resIndex];
                for (auto& nodeSprite : nodeInputRes.children("sprite")) {
                    SpriteData sprite{};
                    sprite.name = nodeSprite.attribute("name").as_string();
                    auto imagePath = nodeSprite.attribute("path").as_string();

                    int w = 0;
                    int h = 0;
                    int ch = 0;
                    auto* pixels = stbi_load(imagePath, &w, &h, &ch, 4);

                    sprite.image = new image_t(w, h, pixels);
                    sprite.source = {0, 0, w, h};
                    sprite.rc.x = nodeSprite.attribute("x").as_float(0.0f);
                    sprite.rc.y = nodeSprite.attribute("y").as_float(0.0f);
                    sprite.rc.width = nodeSprite.attribute("w").as_float(0.0f);
                    sprite.rc.height = nodeSprite.attribute("h").as_float(0.0f);
                    sprite.padding = nodeSprite.attribute("pad").as_uint(1);

                    res.sprites.push_back(sprite);
                }
                ++resIndex;
            }
        }
    }

    {
        working_dir_t workingDir{output};
        atlas.packAndSaveMultiThreaded();
    }
}

}