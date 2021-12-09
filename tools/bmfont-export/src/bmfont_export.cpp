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
    Writer writer{100};
    writer.writeI32(fontData.unitsPerEM);
    writer.writeI32(fontData.fontSize);
    writer.writeI32(fontData.lineHeight);
    writer.writeI32(fontData.ascender);
    writer.writeI32(fontData.descender);
    writer.writeI32((int32_t) fontData.glyphs.size());
    int32_t dictSize = 0;

    for (auto& glyph: fontData.glyphs) {
        for (auto _: glyph.codepoints) {
            ++dictSize;
        }
        writer.writeI32(glyph.box.x);
        writer.writeI32(glyph.box.y);
        writer.writeI32(glyph.box.w);
        writer.writeI32(glyph.box.h);
        writer.writeI32(glyph.advanceWidth);
        writer.writeString(glyph.sprite);
    }
    writer.writeI32(dictSize);
    for (size_t i = 0; i < fontData.glyphs.size(); ++i) {
        const auto& glyph = fontData.glyphs[i];
        for (auto cp: glyph.codepoints) {
            writer.writeU64((int32_t) cp);
            writer.writeI32((int32_t) i);
        }
    }

    auto f = fopen(outputFont, "wb");
    fwrite(writer.data, 1, writer.pos, f);
    fclose(f);

    return 0;
}