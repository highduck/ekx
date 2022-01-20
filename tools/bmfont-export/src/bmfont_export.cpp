#include <ek/hash.h>

#include "bmfont_export/types_impl.h"
#include "bmfont_export/filters_impl.h"
#include "bmfont_export/Bitmap_impl.h"
#include "bmfont_export/BitmapFontBuilder_impl.h"
#include "bmfont_export/Images.h"
#include "bmfont_export/Writer.h"

int main(int argc, char** argv) {

    using namespace bmfont_export;

    if (argc < 2) {
        return 1;
    }

    const char* configPath = argv[1];

    pugi::xml_document xml;
    if (!xml.load_file(configPath)) {
        BMFE_LOG_F("error parse config xml %s\n", configPath);
        return 1;
    }

    auto node = xml.first_child();
    const auto* outputSprites = node.attribute("outputSprites").as_string();
    const auto* outputFont = node.attribute("outputFont").as_string();

    BuildBitmapFontSettings config{};
    config.name = node.attribute("name").as_string();
    config.ttfPath = node.attribute("path").as_string();
    config.readFromXML(node);
    ImageCollection images{};
    for (auto scale: config.resolutions) {
        images.addResolution(scale);
    }

    BMFE_LOG_F("Export Bitmap Font asset: %s\n", config.ttfPath);

    auto fontData = buildBitmapFont(config, images);

    BMFE_LOG_F("Export Bitmap Font images: %s\n", outputSprites);
    save(images, outputSprites);

    BMFE_LOG_F("Export Bitmap Font binary: %s\n", outputFont);
    bytes_writer writer;
    bytes_writer_alloc(&writer, 100);
    bytes_write_i32(&writer, fontData.unitsPerEM);
    bytes_write_i32(&writer, fontData.fontSize);
    bytes_write_i32(&writer, fontData.lineHeight);
    bytes_write_i32(&writer, fontData.ascender);
    bytes_write_i32(&writer, fontData.descender);
    bytes_write_i32(&writer, (int32_t) fontData.glyphs.size());
    int32_t dictSize = 0;

    for (auto& glyph: fontData.glyphs) {
        for (auto _: glyph.codepoints) {
            ++dictSize;
        }
        bytes_write_i32(&writer, glyph.box.x);
        bytes_write_i32(&writer, glyph.box.y);
        bytes_write_i32(&writer, glyph.box.w);
        bytes_write_i32(&writer, glyph.box.h);
        bytes_write_i32(&writer, glyph.advanceWidth);
        bytes_write_u32(&writer, H(glyph.sprite_name.c_str()));
    }
    bytes_write_i32(&writer, dictSize * 2);
    for (uint32_t i = 0; i < fontData.glyphs.size(); ++i) {
        const auto& glyph = fontData.glyphs[i];
        // pair<u32, u32>
        for (auto cp: glyph.codepoints) {
            bytes_write_u32(&writer, cp);
            bytes_write_u32(&writer, i);
        }
    }

    auto f = fopen(outputFont, "wb");
    fwrite(writer.data, 1, writer.pos, f);
    fclose(f);

    bytes_writer_free(&writer);

    return 0;
}