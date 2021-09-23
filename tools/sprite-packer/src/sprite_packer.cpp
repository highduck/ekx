#include "sprite_packer/AtlasPack.h"
#include "sprite_packer/ImageIO_impl.h"

namespace sprite_packer {

int exportAtlas(const char* xmlPath) {
    pugi::xml_document xml;
    if (!xml.load_file(xmlPath)) {
        SPRITE_PACKER_LOG("error parse xml %s\n", xmlPath);
        return 1;
    }
    auto node = xml.first_child();
    const auto* output = node.attribute("output").as_string();
    AtlasConfig atlasConfig{};
    atlasConfig.readFromXML(node);
    Atlas atlas{atlasConfig};

    for (auto& imagesNode: node.children("images")) {
        pugi::xml_document inputDoc;
        if (inputDoc.load_file(imagesNode.attribute("path").as_string())) {
            int resIndex = 0;
            for (auto& nodeInputRes: inputDoc.first_child().children("resolution")) {
                auto& res = atlas.resolutions[resIndex];
                for (auto& nodeSprite: nodeInputRes.children("image")) {
                    SpriteData sprite{};
                    sprite.name = nodeSprite.attribute("name").as_string();
                    auto imagePath = nodeSprite.attribute("path").as_string();

                    int w = 0;
                    int h = 0;
                    int ch = 0;
                    auto* pixels = stbi_load(imagePath, &w, &h, &ch, 4);

                    sprite.bitmap = new Bitmap(pixels, w, h);
                    sprite.source = {0, 0, w, h};
                    sprite.rc.x = nodeSprite.attribute("x").as_float(0.0f);
                    sprite.rc.y = nodeSprite.attribute("y").as_float(0.0f);
                    sprite.rc.w = nodeSprite.attribute("w").as_float(0.0f);
                    sprite.rc.h = nodeSprite.attribute("h").as_float(0.0f);
                    sprite.padding = nodeSprite.attribute("p").as_uint(1);

                    res.sprites.push_back(sprite);
                }
                ++resIndex;
            }
        }
    }

    atlas.packAndSaveMultiThreaded(output);
    return 0;
}

}

int main(int argc, char** argv) {
    if(argc < 2) {
        return 1;
    }

    const char* configPath = argv[1];
    return sprite_packer::exportAtlas(configPath);
}