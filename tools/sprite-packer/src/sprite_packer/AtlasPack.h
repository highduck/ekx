#pragma once

#include <ek/io.h>

#include "Atlas.h"
#include <pugixml.hpp>
#include <thread>
#include "binpack.h"
#include "sprpk_image.h"
#include <ek/log.h>
#include <ek/print.h>
#include <ek/hash.h>
#include <cstdio>

namespace sprite_packer {

std::vector<PageData> packSprites(std::vector<SpriteData> sprites, int maxWidth, int maxHeight);

void formatAtlasFileName(char* buffer, int bufferSize, const char* name, float scale, int pageIndex, const char* ext) {
    char pageSuffix[8];
    *pageSuffix = 0;
    if (pageIndex != 0) {
        ek_snprintf(pageSuffix, 8, "_%d", pageIndex);
    }
    const char* scaleSuffix = "";
    if (scale <= 1) scaleSuffix = "@1x";
    else if (scale <= 2) scaleSuffix = "@2x";
    else if (scale <= 3) scaleSuffix = "@3x";
    else if (scale <= 4) scaleSuffix = "@4x";
    else {
        log_warn("atlas more than 4x scale-factor! %d %%\n", (int) (100 * scale));
        // to support bigger density
        scaleSuffix = "@4x";
    }
    ek_snprintf(buffer, bufferSize, "%s%s%s.%s", name, pageSuffix, scaleSuffix, ext);
}

void save_atlas_resolution(AtlasData& resolution, const char* outputPath, const char* name) {
    io_t io;
    io_alloc(&io, 100);
    io_write_i32(&io, (int32_t) resolution.pages.size());

    char imagePath[1024];
    int page_index = 0;
    for (auto& page: resolution.pages) {
        EK_ASSERT(page.bitmap.pixels != nullptr);
        formatAtlasFileName(imagePath, 1024, name, resolution.resolution_scale, page_index, "png");
        IO_WRITE(&io, page.w);
        IO_WRITE(&io, page.h);
        io_write_string(&io, imagePath, (int) strnlen(imagePath, 1024));
        io_write_u32(&io, page.sprites.size());
        for (auto& spr: page.sprites) {
            string_hash_t sprite_name = H(spr.name.c_str());
            // keep only rotation flag in output
            uint32_t sprite_flags = spr.flags & 1;
            IO_WRITE(&io, sprite_name);
            IO_WRITE(&io, sprite_flags);
            IO_WRITE(&io, spr.rc);
            IO_WRITE(&io, spr.uv);
        }

        char absImagePath[1024];
        ek_snprintf(absImagePath, 1024, "%s/%s", outputPath, imagePath);

        //page.image_path = name + get_atlas_suffix(resolution.resolution_scale, page_index) + ".png";
        sprite_pack_image_save(&page.bitmap, absImagePath, SPRITE_PACK_ALPHA | SPRITE_PACK_PNG);
        // saveImageJPG(*page.image, name + get_atlas_suffix(resolution.resolution_scale, page_index));
        ++page_index;
    }

    char atlasPath[1024];
    formatAtlasFileName(atlasPath, 1024, name, resolution.resolution_scale, 0, "atlas");
    char absAtlasPath[1024];
    ek_snprintf(absAtlasPath, 1024, "%s/%s", outputPath, atlasPath);
    auto f = fopen(absAtlasPath, "wb");
    fwrite(io.data, 1, io.pos, f);
    fclose(f);

    io_free(&io);
}

static void packAtlasThread(const char* name, const char* outputPath, AtlasData& resolution) {
    resolution.pages = packSprites(resolution.sprites, resolution.maxWidth, resolution.maxHeight);
    log_info("  - '%s-%d' packed\n", name, resolution.resolution_index);
    save_atlas_resolution(resolution, outputPath, name);
    log_info("  - '%s-%d' encoded\n", name, resolution.resolution_index);
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
    log_info("'%s' atlas build completed\n", name.c_str());
}

/*** Pack Sprites ***/

irect_t no_pack_padding(binpack_rect_t rect, int pad) {
    rect.x += pad;
    rect.y += pad;
    rect.r -= pad;
    rect.b -= pad;
    return {{rect.x, rect.y, rect.r - rect.x, rect.b - rect.y}};
}

rect_t calc_uv(irect_t source, float tex_w, float tex_h, bool rotated) {
    if (rotated) {
        const int sw = source.w;
        source.w = source.h;
        source.h = sw;
    }
    return {{
                    (float) source.x / tex_w,
                    (float) source.y / tex_h,
                    (float) source.w / tex_w,
                    (float) source.h / tex_h
            }};
}

std::vector<PageData> packSprites(std::vector<SpriteData> sprites, const int maxWidth, const int maxHeight) {
    std::vector<PageData> pages;

//    timer timer{};
//    log_debug("Packing %lu sprites...", sprites.size());
    binpack_t binpack;
    bool need_to_pack = true;
    while (need_to_pack) {
        binpack_init(&binpack, maxWidth, maxHeight);

        for (auto& sprite: sprites) {
            if (!(sprite.flags & SPRITE_FLAG_PACKED)) {
                binpack_add(&binpack,
                            sprite.source.w,
                            sprite.source.h,
                            sprite.padding,
                            &sprite);
            }
        }

        // check if any rects added to pack
        need_to_pack = !!binpack.rects_num;
        if (need_to_pack) {
            binpack_run(&binpack);

            PageData page{};
            page.w = binpack.canvas.w;
            page.h = binpack.canvas.h;
            bitmap_alloc(&page.bitmap, page.w, page.h);
            const auto fw = (float) page.w;
            const auto fh = (float) page.h;

            {
                //atlas_renderer_cairo renderer{*page.image};
                for (uint32_t i = 0; i < binpack.rects_num; ++i) {
                    if (!(binpack.flags[i] & BINPACK_PACKED)) {
                        continue;
                    }
                    auto* sprite = (SpriteData*) binpack.user_data[i];
                    sprite->flags |= SPRITE_FLAG_PACKED;

                    irect_t packed_rect = no_pack_padding(binpack.rects[i], sprite->padding);
                    if (binpack.flags[i] & BINPACK_ROTATED) {
                        sprite->flags |= SPRITE_FLAG_ROTATED;
                        bitmap_blit_copy_ccw90(&page.bitmap, packed_rect.x, packed_rect.y, &sprite->bitmap,
                                               sprite->source.x, sprite->source.y, sprite->source.w, sprite->source.h);
                    } else {
                        sprite->flags &= ~SPRITE_FLAG_ROTATED;
                        bitmap_blit_copy(&page.bitmap, packed_rect.x, packed_rect.y, &sprite->bitmap,
                                         sprite->source.x, sprite->source.y, sprite->source.w, sprite->source.h);
                    }
                    bitmap_free(&sprite->bitmap);
                    sprite->bitmap = page.bitmap;
                    // now packed_rect is rotated, so do not copy the original rect dimensions
                    sprite->source = packed_rect;
                    sprite->uv = calc_uv(packed_rect, fw, fh, sprite->flags & SPRITE_FLAG_ROTATED );

                    page.sprites.emplace_back(*sprite);
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