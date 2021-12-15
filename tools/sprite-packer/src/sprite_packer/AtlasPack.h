#pragma once

#include "Atlas.h"
#include <pugixml.hpp>
#include <thread>
#include "MaxRects.hpp"
#include "sprpk_image.h"
#include "Writer.h"
#include <cstdio>

namespace sprite_packer {

std::vector<PageData> packSprites(std::vector<SpriteData> sprites, int maxWidth, int maxHeight);

void formatAtlasFileName(char* buffer, int bufferSize, const char* name, float scale, int pageIndex, const char* ext) {
    char pageSuffix[8];
    *pageSuffix = 0;
    if (pageIndex != 0) {
        snprintf(pageSuffix, 8, "_%d", pageIndex);
    }
    const char* scaleSuffix = "";
    if (scale <= 1) scaleSuffix = "@1x";
    else if (scale <= 2) scaleSuffix = "@2x";
    else if (scale <= 3) scaleSuffix = "@3x";
    else if (scale <= 4) scaleSuffix = "@4x";
    else {
        SPRITE_PACKER_LOG("atlas more than 4x scale-factor! %d %%\n", (int) (100 * scale));
        // to support bigger density
        scaleSuffix = "@4x";
    }
    snprintf(buffer, bufferSize, "%s%s%s.%s", name, pageSuffix, scaleSuffix, ext);
}

void save_atlas_resolution(AtlasData& resolution, const char* outputPath, const char* name) {
    bytes_writer writer;
    bytes_writer_alloc(&writer, 100);
    bytes_write_i32(&writer, (int32_t) resolution.pages.size());

    char imagePath[1024];
    int page_index = 0;
    for (auto& page: resolution.pages) {
        assert(page.bitmap.pixels != nullptr);
        formatAtlasFileName(imagePath, 1024, name, resolution.resolution_scale, page_index, "png");
        bytes_write_u16(&writer, page.w);
        bytes_write_u16(&writer, page.h);
        bytes_write_string(&writer, imagePath, (int) strnlen(imagePath, 1024));
        bytes_write_i32(&writer, (int32_t) page.sprites.size());
        for (auto& spr: page.sprites) {
            bytes_write_string(&writer, spr.name.c_str(), (int) spr.name.size());
            bytes_writer_push(&writer, &spr.rc, sizeof(spr.rc));
            bytes_writer_push(&writer, &spr.uv, sizeof(spr.uv));
            // keep only rotation flag in output
            bytes_write_u8(&writer, spr.flags & 1);
        }

        char absImagePath[1024];
        snprintf(absImagePath, 1024, "%s/%s", outputPath, imagePath);

        //page.image_path = name + get_atlas_suffix(resolution.resolution_scale, page_index) + ".png";
        sprite_pack_image_save(&page.bitmap, absImagePath, SPRITE_PACK_ALPHA | SPRITE_PACK_PNG);
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

    bytes_writer_free(&writer);
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

ek_img_rect no_pack_padding(ek::binpack::rect_t rect, int pad) {
    return {
            rect.x + pad,
            rect.y + pad,
            rect.width - 2 * pad,
            rect.height - 2 * pad
    };
}

ek_img_rect_f calc_uv(ek_img_rect source, float w, float h, bool rotated) {
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
//    EK_DEBUG("Packing %lu sprites...", sprites.size());

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
            ek_bitmap_alloc(&page.bitmap, page.w, page.h);
            const auto fw = (float) page.w;
            const auto fh = (float) page.h;

            {
                //atlas_renderer_cairo renderer{*page.image};
                for (int i = 0; i < (int)packer_state.rects.size(); ++i) {
                    if (!packer_state.is_packed(i)) {
                        continue;
                    }
                    auto& sprite = packer_state.get_user_data<SpriteData>(i);
                    sprite.enable(SpriteFlag::Packed);
                    if (packer_state.is_rotated(i)) {
                        sprite.enable(SpriteFlag::Rotated);
                    }

                    const ek_img_rect packed_rect = no_pack_padding(packer_state.get_rect(i), sprite.padding);
                    if (sprite.is_rotated()) {
                        copyPixels_CCW_90(&page.bitmap, packed_rect.x, packed_rect.y, &sprite.bitmap,
                                          sprite.source.x, sprite.source.y, sprite.source.w, sprite.source.h);
                    } else {
                        copyPixels(&page.bitmap, packed_rect.x, packed_rect.y, &sprite.bitmap,
                                   sprite.source.x, sprite.source.y, sprite.source.w, sprite.source.h);
                    }
                    ek_bitmap_free(&sprite.bitmap);
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

}