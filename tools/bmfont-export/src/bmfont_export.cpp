#include <ek/hash.h>
#include <ek/io.h>
#include <ek/log.h>
#include <ek/format/bmfont.h>

#include "bmfont_export/types_impl.h"
#include "bmfont_export/filters_impl.h"
#include "bmfont_export/Bitmap_impl.h"
#include "bmfont_export/BitmapFontBuilder_impl.h"
#include "bmfont_export/Images.h"

int main(int argc, char** argv) {

    using namespace bmfont_export;

    if (argc < 2) {
        return 1;
    }

    const char* configPath = argv[1];

    pugi::xml_document xml;
    if (!xml.load_file(configPath)) {
        log_error("error parse config xml %s\n", configPath);
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

    log_info("Export Bitmap Font asset: %s", config.ttfPath);

    auto fontData = buildBitmapFont(config, images);

    log_info("Export Bitmap Font images: %s", outputSprites);
    save(images, outputSprites);

    log_info("Export Bitmap Font binary: %s", outputFont);

    // calculate codepoint map size
    uint32_t dictSize = 0;
    for (auto& glyph: fontData.glyphs) {
        dictSize += glyph.codepoints.size();
    }

    io_t io;
    io_alloc(&io, 100);

    bmfont_header hdr;
    hdr.base_font_size = fontData.fontSize;
    hdr.line_height_multiplier = fontData.lineHeight;
    hdr.ascender = fontData.ascender;
    hdr.descender = fontData.descender;
    hdr.codepoints_data_offset = (sizeof hdr);
    hdr.codepoints_num = dictSize;
    hdr.glyphs_data_offset = hdr.codepoints_data_offset + sizeof(uint32_t) * dictSize * 2;
    hdr.glyphs_num = fontData.glyphs.size();
   io_push(&io, &hdr, sizeof hdr);

    // codepoints map
    for (uint32_t i = 0; i < fontData.glyphs.size(); ++i) {
        const auto& glyph = fontData.glyphs[i];
        // pair<u32, u32>
        for (auto cp: glyph.codepoints) {
            io_write_u32(&io, cp);
            io_write_u32(&io, i);
        }
    }

    // glyphs
    for (auto& glyph: fontData.glyphs) {
        io_push(&io, &glyph.box, sizeof glyph.box);
        io_write_f32(&io, glyph.advance_x);
        io_write_u32(&io, H(glyph.sprite_name.c_str()));
    }

    auto f = fopen(outputFont, "wb");
    fwrite(io.data, 1, io.pos, f);
    fclose(f);

    io_free(&io);

    return 0;
}