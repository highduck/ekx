#include "sprite_packer/AtlasPack.h"
#include <ek/log.h>

namespace sprite_packer {

uint32_t* cast_align_4(void* p) {
    EK_ASSERT(((uintptr_t) p % 4u) == 0);
    return (uint32_t*) p;
}

bitmap_t load_bitmap32(const char* filepath) {
    bitmap_t result = {};
    FILE* stream = fopen(filepath, "rb");
    if (!stream) {
        return result;
    }
    fseek(stream, 0, SEEK_END);
    size_t size = (size_t) ftell(stream);
    uint8_t* buf = (uint8_t*) malloc(size);
    fseek(stream, 0, SEEK_SET);
    fread(buf, size, 1u, stream);
    bool success = ferror(stream) == 0;
    fclose(stream);
    if (success) {
        int channels = 0;
        void* pixels = stbi_load_from_memory(buf, (int) (size & 0x7FFFFFFFu), &result.w, &result.h, &channels,
                                                STBI_rgb_alpha);
        result.pixels = (color_t*)pixels;
        //result.pixels = (color_t*)cast_align_4(pixels);
    }
    free(buf);
    return result;
}

int exportAtlas(const char* xmlPath) {
    pugi::xml_document xml;
    if (!xml.load_file(xmlPath)) {
        log_error("error parse xml: %s", xmlPath);
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

                    sprite.bitmap = load_bitmap32(imagePath);
                    if (sprite.bitmap.pixels) {
                        int w = sprite.bitmap.w;
                        int h = sprite.bitmap.h;
                        sprite.source = {{0, 0, w, h}};
                        sprite.rc.x = nodeSprite.attribute("x").as_float(0.0f);
                        sprite.rc.y = nodeSprite.attribute("y").as_float(0.0f);
                        sprite.rc.w = nodeSprite.attribute("w").as_float(0.0f);
                        sprite.rc.h = nodeSprite.attribute("h").as_float(0.0f);
                        sprite.padding = nodeSprite.attribute("p").as_uint(1);
                        if (nodeSprite.attribute("trim").as_bool(true)) {
                            sprite.flags |= SPRITE_FLAG_TRIM;
                        }
                        res.sprites.push_back(sprite);
                    } else {
                        EK_ASSERT(!"failed to load bitmap");
                    }
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
    if (argc < 2) {
        return 1;
    }

    const char* configPath = argv[1];
    return sprite_packer::exportAtlas(configPath);
}