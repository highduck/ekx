#pragma once

#include "Atlas.h"
#include <pugixml.hpp>
#include <thread>
#include "max_rects.hpp"
#include "ImageIO.h"
#include "Bitmap.h"
#include "Writer.h"
#include <cstdio>

namespace sprite_packer {

void saveImagePNG(const Bitmap& bitmap, const char* path, bool alpha);

std::vector<PageData> packSprites(std::vector<SpriteData> sprites, int maxWidth, int maxHeight);

void formatAtlasFileName(char* buffer, int bufferSize, const char* name, float scale, int pageIndex, const char* ext) {
    char pageSuffix[8] = "";
    if (pageIndex != 0) {
        snprintf(pageSuffix, 8, "_%d", pageIndex);
    }
    const char* scaleSuffix = "";
    if (scale <= 1) scaleSuffix = "@1x";
    else if (scale <= 2) scaleSuffix = "@2x";
    else if (scale <= 3) scaleSuffix = "@3x";
    else if (scale <= 4) scaleSuffix = "@4x";
    else {
        SPRITE_PACKER_LOG("atlas more than 4x scale-factor! %d %%\n", (int)(100 * scale));
        // to support bigger density
        scaleSuffix = "@4x";
    }
    snprintf(buffer, bufferSize, "%s%s%s.%s", name, pageSuffix, scaleSuffix, ext);
}

void save_atlas_resolution(AtlasData& resolution, const char* outputPath, const char* name) {
    Writer writer{100};
    writer.writeI32((int32_t) resolution.pages.size());

    int page_index = 0;
    for (auto& page: resolution.pages) {
        assert(page.bitmap != nullptr);
        char imagePath[1024];
        formatAtlasFileName(imagePath, 1024, name, resolution.resolution_scale, page_index, "png");
        writer.writeU16(page.w);
        writer.writeU16(page.h);
        writer.writeString(std::string{imagePath});
        writer.writeI32((int32_t) page.sprites.size());
        for (auto& spr: page.sprites) {
            writer.writeString(spr.name);
            writer.writeF32(spr.rc.x);
            writer.writeF32(spr.rc.y);
            writer.writeF32(spr.rc.w);
            writer.writeF32(spr.rc.h);
            writer.writeF32(spr.uv.x);
            writer.writeF32(spr.uv.y);
            writer.writeF32(spr.uv.w);
            writer.writeF32(spr.uv.h);
            writer.writeU8(spr.flags);
        }

        char absImagePath[1024];
        snprintf(absImagePath, 1024, "%s/%s", outputPath, imagePath);

        //page.image_path = name + get_atlas_suffix(resolution.resolution_scale, page_index) + ".png";
        saveImagePNG(*page.bitmap, absImagePath, true);
        // saveImageJPG(*page.image, name + get_atlas_suffix(resolution.resolution_scale, page_index));
        ++page_index;
    }

    char atlasPath[1024];
    formatAtlasFileName(atlasPath, 1024, name, resolution.resolution_scale, 0, "atlas");
    char absAtlasPath[1024];
    snprintf(absAtlasPath, 1024, "%s/%s", outputPath, atlasPath);
    auto f = fopen(absAtlasPath, "wb");
    fwrite(writer.data, 1, writer.pos, f);
    fclose(f);
}

static void packAtlasThread(const char* name, const char* outputPath, AtlasData& resolution) {
    resolution.pages = packSprites(resolution.sprites, resolution.maxWidth, resolution.maxHeight);
    SPRITE_PACKER_LOG("  - '%s-%d' packed\n", name, resolution.resolution_index);
    save_atlas_resolution(resolution, outputPath, name);
    SPRITE_PACKER_LOG("  - '%s-%d' encoded\n", name, resolution.resolution_index);
}

void Atlas::packAndSaveMultiThreaded(const char* outputPath) {
    std::vector<std::thread> threads;
    threads.reserve(resolutions.size());

    for (auto& resolution: resolutions) {
        threads.emplace_back(&packAtlasThread, name.c_str(), outputPath, std::ref(resolution));
    }

    for (auto& th: threads) {
        th.join();
    }
    SPRITE_PACKER_LOG("'%s' atlas build completed\n", name.c_str());
}

/*** Pack Sprites ***/

RectI no_pack_padding(ek::binpack::rect_t rect, int pad) {
    return {
            rect.x + pad,
            rect.y + pad,
            rect.width - 2 * pad,
            rect.height - 2 * pad
    };
}

Rect calc_uv(RectI source, float w, float h, bool rotated) {
    if (rotated) {
        std::swap(source.w, source.h);
    }
    return {
            (float) source.x / w,
            (float) source.y / h,
            (float) source.w / w,
            (float) source.h / h
    };
}


std::vector<PageData> packSprites(std::vector<SpriteData> sprites, const int maxWidth, const int maxHeight) {
    std::vector<PageData> pages;

//    timer timer{};
//    EK_DEBUG_F("Packing %lu sprites...", sprites.size());

    bool need_to_pack = true;
    while (need_to_pack) {
        ek::binpack::packer_state_t packer_state{maxWidth, maxHeight};

        for (auto& sprite: sprites) {
            if (!sprite.is_packed()) {
                packer_state.add(
                        sprite.source.w,
                        sprite.source.h,
                        sprite.padding,
                        &sprite);
            }
        }

        need_to_pack = !packer_state.empty();
        if (need_to_pack) {
            pack_nodes(packer_state);

            PageData page{};
            page.w = packer_state.canvas.width;
            page.h = packer_state.canvas.height;
            page.bitmap = new Bitmap(page.w, page.h);
            const auto fw = (float) page.w;
            const auto fh = (float) page.h;

            {
                //atlas_renderer_cairo renderer{*page.image};
                for (size_t i = 0; i < packer_state.rects.size(); ++i) {
                    if (!packer_state.is_packed(i)) {
                        continue;
                    }
                    auto& sprite = packer_state.get_user_data<SpriteData>(i);
                    sprite.enable(SpriteFlag::Packed);
                    if (packer_state.is_rotated(i)) {
                        sprite.enable(SpriteFlag::Rotated);
                    }

                    const RectI packed_rect = no_pack_padding(packer_state.get_rect(i), sprite.padding);
                    if (sprite.is_rotated()) {
                        copyPixels_CCW_90(*page.bitmap, packed_rect.x, packed_rect.y, *sprite.bitmap,
                                          sprite.source.x, sprite.source.y, sprite.source.w, sprite.source.h);
                    } else {
                        copyPixels(*page.bitmap, packed_rect.x, packed_rect.y, *sprite.bitmap,
                                   sprite.source.x, sprite.source.y, sprite.source.w, sprite.source.h);
                    }
                    delete sprite.bitmap;
                    sprite.bitmap = page.bitmap;
                    sprite.source = packed_rect;
                    sprite.uv = calc_uv(packed_rect, fw, fh, sprite.is_rotated());

                    page.sprites.emplace_back(sprite);
                }
            }
            pages.emplace_back(std::move(page));
        }
    }

//    LOG_PERF("Packed %lu to %lu pages for %lf ms",
//             sprites.size(),
//             pages.size(),
//             get_elapsed_time(timer));

    return pages;
}

// TODO: atlas
// ++page_index;
// page.image_path = atlas.name + get_atlas_suffix(atlas.scale, page_index) + ".png";

/*** Save Image ***/

void saveImagePNG(const Bitmap& bitmap, const char* path, bool alpha) {
    stbi_write_png_compression_level = 10;
    stbi_write_force_png_filter = 0;

    if (alpha) {
        stbi_write_png(path, bitmap.w, bitmap.h, 4, bitmap.data, 4 * bitmap.w);
    } else {
        Bitmap img{bitmap};
        size_t pixels_count = img.w * img.h;
        uint8_t* buffer = (uint8_t*) malloc(pixels_count * 3);
        uint8_t* buffer_rgb = buffer;
        uint8_t* buffer_rgba = (uint8_t*) img.data;

        for (size_t i = 0; i < pixels_count; ++i) {
            buffer_rgb[0] = buffer_rgba[0];
            buffer_rgb[1] = buffer_rgba[1];
            buffer_rgb[2] = buffer_rgba[2];
            buffer_rgba += 4;
            buffer_rgb += 3;
        }

        stbi_write_png(path,
                       img.w,
                       img.h,
                       3,
                       buffer,
                       3 * img.w);

        free(buffer);
    }
}

void saveImageJPG(const Bitmap& bitmap, const std::string& path, bool alpha) {
    Bitmap img{bitmap};
    // require RGBA non-premultiplied alpha
    //undo_premultiply_image(img);

    if (alpha) {
        size_t pixels_count = img.w * img.h;
        uint8_t* buffer_rgb = (uint8_t*) malloc(pixels_count * 3);
        uint8_t* buffer_alpha = (uint8_t*) malloc(pixels_count);
        uint8_t* buffer_rgba = (uint8_t*)img.data;

        uint8_t* rgb = buffer_rgb;
        uint8_t* alphaMask = buffer_alpha;
        for (size_t i = 0; i < pixels_count; ++i) {
            rgb[0] = buffer_rgba[0];
            rgb[1] = buffer_rgba[1];
            rgb[2] = buffer_rgba[2];
            alphaMask[0] = buffer_rgba[3];
            buffer_rgba += 4;
            rgb += 3;
            alphaMask += 1;
        }

        stbi_write_jpg((path + ".jpg").c_str(),
                       img.w,
                       img.h,
                       3,
                       buffer_rgb,
                       90);

        stbi_write_jpg((path + "a.jpg").c_str(),
                       img.w,
                       img.h,
                       1,
                       buffer_alpha,
                       90);

        free(buffer_rgb);
        free(buffer_alpha);
    } else {

        size_t pixels_count = img.w * img.h;
        auto* buffer = (uint8_t*) malloc(pixels_count * 3);
        auto* buffer_rgb = buffer;
        auto* buffer_rgba = (uint8_t*) img.data;

        for (size_t i = 0; i < pixels_count; ++i) {
            buffer_rgb[0] = buffer_rgba[0];
            buffer_rgb[1] = buffer_rgba[1];
            buffer_rgb[2] = buffer_rgba[2];
            buffer_rgba += 4;
            buffer_rgb += 3;
        }

        stbi_write_jpg(path.c_str(),
                       img.w,
                       img.h,
                       3,
                       buffer,
                       3 * img.w);

        free(buffer);
    }
}

}