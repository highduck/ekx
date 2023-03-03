#include <ek/hash.h>
#include <ek/log.h>
#include <ek/format/bmfont.h>
#include <ek/print.h>

#include "bmfont_export/build_bitmap_font.c.h"

int export_bitmap_font(const char* config_path) {
    FILE* f = fopen(config_path, "rb");
    if (!f) {
        log_error("error parse config %s", config_path);
        return 1;
    }

    char output_sprites[1024];
    char output_font[1024];
    char font_name[1024];
    char ttf_path[1024];
    fscanf(f, "%s\n%s\n%s\n%s\n", output_sprites, output_font, font_name, ttf_path);

    bitmap_font_build_options_t options = {0};
    options.ttf_path = ttf_path;
    parse_bitmap_font_build_options(f, &options);
    image_set_t images = {0};
    images.resolutions_num = options.resolutions_num;
    for (uint32_t i = 0; i < options.resolutions_num; ++i) {
        images.resolutions[i] = (resolution_t) {
                .index = i, .scale = options.resolutions[i]
        };
    }
    fclose(f);

    log_info("Export Bitmap Font asset: %s", options.ttf_path);

    const font_t fontData = build_bitmap_font(&options, &images);

    log_info("Export Bitmap Font images: %s", output_sprites);
    save_font_bitmaps(font_name, &images, output_sprites);

    log_info("Export Bitmap Font binary: %s", output_font);

    // calculate codepoint map size
    uint32_t dictSize = 0;
    uint32_t glyphs_num = ek_buf_length(fontData.glyphs);
    for (uint32_t i = 0; i < glyphs_num; ++i) {
        dictSize += ek_buf_length(fontData.glyphs[i].codepoints);
    }

    f = fopen(output_font, "wb");
    bmfont_header hdr = {0};
    hdr.base_font_size = fontData.fontSize;
    hdr.line_height_multiplier = fontData.lineHeight;
    hdr.ascender = fontData.ascender;
    hdr.descender = fontData.descender;
    hdr.codepoints_data_offset = (sizeof hdr);
    hdr.codepoints_num = dictSize;
    hdr.glyphs_data_offset = hdr.codepoints_data_offset + sizeof(uint32_t) * dictSize * 2;
    hdr.glyphs_num = glyphs_num;
    fwrite(&hdr, 1, sizeof hdr, f);

    // codepoints map
    for (uint32_t i = 0; i < glyphs_num; ++i) {
        const glyph_t* glyph = fontData.glyphs + i;
        // pair<u32, u32>
        const uint32_t codepoints_num = ek_buf_length(glyph->codepoints);
        for (uint32_t ci = 0; ci < codepoints_num; ++ci) {
            uint32_t cp = glyph->codepoints[ci];
            fwrite(&cp, 1, sizeof(uint32_t), f);
            fwrite(&i, 1, sizeof(uint32_t), f);
        }
    }

    char sprite_name[1024];

    // glyphs
    for (uint32_t i = 0; i < glyphs_num; ++i) {
        const glyph_t* glyph = fontData.glyphs + i;
        fwrite(&glyph->box, 1, sizeof glyph->box, f);
        fwrite(&glyph->advance_x, 1, sizeof(float), f);
        ek_snprintf(sprite_name, sizeof sprite_name, "%s%u", font_name, glyph->glyph_index);
        const uint32_t hashed_name = H(sprite_name);
        fwrite(&hashed_name, 1, sizeof(uint32_t), f);
    }

    fclose(f);

    return 0;
}

int main(int argc, char** argv) {
    log_init();
    if (argc < 2) {
        return 1;
    }
    const char* command = argv[1];
    log_info("run command %s", command);
    if (!strcmp("bmfont-export", command)) {
        if (argc < 3) {
            return 1;
        }
        const char* config_path = argv[2];
        return export_bitmap_font(config_path);
    }
    return 0;
}
