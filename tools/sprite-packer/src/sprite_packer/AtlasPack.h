#pragma once

#include <ek/io.h>

#include "Atlas.h"
#include "binpack.h"
#include "sprpk_image.h"
#include <ek/log.h>
#include <ek/print.h>
#include <ek/hash.h>
#include <stdio.h>
#include <pthread.h>

uint32_t pack_sprites(sprite_data_t* sprites, uint32_t sprites_num, uint16_t max_width, uint16_t max_height,
                      atlas_page_t* out_pages, uint32_t max_pages);

void
formatAtlasFileName(char* buffer, int bufferSize, const char* name, float scale, uint32_t page_index, const char* ext) {
    char pageSuffix[8];
    *pageSuffix = 0;
    if (page_index != 0) {
        ek_snprintf(pageSuffix, 8, "_%d", page_index);
    }
    const char* scaleSuffix = "";
    if (scale <= 1) scaleSuffix = "1x";
    else if (scale <= 2) scaleSuffix = "2x";
    else if (scale <= 3) scaleSuffix = "3x";
    else if (scale <= 4) scaleSuffix = "4x";
    else {
        log_warn("atlas more than 4x scale-factor! %d %%", (int) (100 * scale));
        // to support bigger density
        scaleSuffix = "4x";
    }
    ek_snprintf(buffer, bufferSize, "%s%s%s.%s", name, pageSuffix, scaleSuffix, ext);
}

void save_atlas_resolution(atlas_res_t* resolution, const char* output_path, const char* name) {
    io_t io;
    io_alloc(&io, 100);
    io_write_i32(&io, (int32_t) resolution->pages_num);

    char image_path[1024];
    for (uint32_t page_index = 0; page_index < resolution->pages_num; ++page_index) {
        atlas_page_t* page = resolution->pages + page_index;
        EK_ASSERT(page->bitmap.pixels != NULL);
        formatAtlasFileName(image_path, 1024, name, resolution->resolution.scale, page_index, "png");
        IO_WRITE(&io, page->w);
        IO_WRITE(&io, page->h);
        io_write_string(&io, image_path, (int) strnlen(image_path, 1024));
        io_write_u32(&io, page->sprites_num);
        for (uint32_t spr_index = 0; spr_index < page->sprites_num; ++spr_index) {
            sprite_data_t* spr = page->sprites + spr_index;
            IO_WRITE(&io, spr->name);
            // keep only rotation flag in output
            io_write_u32(&io, spr->flags & 1);
            IO_WRITE(&io, spr->rc);
            IO_WRITE(&io, spr->uv);
        }

        char absImagePath[1024];
        ek_snprintf(absImagePath, 1024, "%s/%s", output_path, image_path);

        //page.image_path = name + get_atlas_suffix(resolution.resolution_scale, page_index) + ".png";
        sprite_pack_image_save(&page->bitmap, absImagePath, SPRITE_PACK_ALPHA | SPRITE_PACK_PNG);
        // saveImageJPG(*page.image, name + get_atlas_suffix(resolution.resolution_scale, page_index));
    }

    char atlas_path[1024];
    formatAtlasFileName(atlas_path, 1024, name, resolution->resolution.scale, 0, "atlas");
    char abs_atlas_path[1024];
    ek_snprintf(abs_atlas_path, 1024, "%s/%s", output_path, atlas_path);
    FILE* f = fopen(abs_atlas_path, "wb");
    fwrite(io.data, 1, io.pos, f);
    fclose(f);

    io_free(&io);
}

typedef struct {
    const char* atlas_name;
    const char* output_path;
    atlas_res_t* atlas_res;
} pack_thread_args;

void* packAtlasThread(void* data) {
    pack_thread_args* args = (pack_thread_args*) data;
    atlas_res_t* resolution = args->atlas_res;
    const char* name = args->atlas_name;
    const char* output_path = args->output_path;
    resolution->pages_num = pack_sprites(resolution->sprites, resolution->sprites_num,
                                         resolution->resolution.max_width, resolution->resolution.max_height,
                                         resolution->pages, 32);
    if (resolution->pages_num) {
        int scale_percents = (int) (resolution->resolution.scale * 100);
        log_info("  - '%s-%d' packed", name, scale_percents);
        save_atlas_resolution(resolution, args->output_path, name);
        log_info("  - '%s-%d' encoded", name, scale_percents);
    } else {
        log_warn("  - %s - no pages!", name);
    };
}


/*** Pack Sprites ***/

irect_t no_pack_padding(binpack_rect_t rect, int pad) {
    rect.x += pad;
    rect.y += pad;
    rect.r -= pad;
    rect.b -= pad;
    return (irect_t){
        .x = rect.x,
        .y = rect.y,
        .w = rect.r - rect.x,
        .h = rect.b - rect.y
    };
}

rect_t calc_uv(irect_t source, float tex_w, float tex_h, bool rotated) {
    if (rotated) {
        const int sw = source.w;
        source.w = source.h;
        source.h = sw;
    }
    rect_t result = irect_to_rect(source);
    result.x /= tex_w;
    result.y /= tex_h;
    result.w /= tex_w;
    result.h /= tex_h;
    return result;
}

uint32_t pack_sprites(sprite_data_t* sprites, uint32_t sprites_num, const uint16_t max_width, const uint16_t max_height,
                      atlas_page_t* out_pages, uint32_t max_pages) {
//    timer timer{};
//    log_debug("Packing %lu sprites...", sprites.size());
    binpack_t binpack;
    uint32_t current_page = 0;
    bool need_to_pack = true;
    while (need_to_pack) {
        binpack_init(&binpack, max_width, max_height);

        for (uint32_t i = 0; i < sprites_num; ++i) {
            sprite_data_t* sprite = sprites + i;
            if (!(sprite->flags & SPRITE_FLAG_PACKED)) {
                binpack_add(&binpack,
                            sprite->source.w,
                            sprite->source.h,
                            sprite->padding,
                            sprite);
            }
        }

        // check if any rects added to pack
        need_to_pack = !!binpack.rects_num;
        if (need_to_pack) {
            binpack_run(&binpack);

            atlas_page_t* page = out_pages + current_page;
            page->w = binpack.canvas.w;
            page->h = binpack.canvas.h;
            bitmap_alloc(&page->bitmap, page->w, page->h);
            const float fw = (float) page->w;
            const float fh = (float) page->h;
            uint32_t packed_count = 0;
            for (uint32_t i = 0; i < binpack.rects_num; ++i) {
                if (binpack.flags[i] & BINPACK_PACKED) {
                    ++packed_count;
                }
            }
            page->sprites = (sprite_data_t*) malloc(sizeof(sprite_data_t) * packed_count);
            page->sprites_num = 0;

            for (uint32_t i = 0; i < binpack.rects_num; ++i) {
                if (!(binpack.flags[i] & BINPACK_PACKED)) {
                    continue;
                }
                sprite_data_t* sprite = (sprite_data_t*) binpack.user_data[i];
                sprite->flags |= SPRITE_FLAG_PACKED;

                irect_t packed_rect = no_pack_padding(binpack.rects[i], sprite->padding);
                if (binpack.flags[i] & BINPACK_ROTATED) {
                    sprite->flags |= SPRITE_FLAG_ROTATED;
                    bitmap_blit_copy_ccw90(&page->bitmap, packed_rect.x, packed_rect.y, &sprite->bitmap,
                                           sprite->source.x, sprite->source.y, sprite->source.w, sprite->source.h);
                } else {
                    sprite->flags &= ~SPRITE_FLAG_ROTATED;
                    bitmap_blit_copy(&page->bitmap, packed_rect.x, packed_rect.y, &sprite->bitmap,
                                     sprite->source.x, sprite->source.y, sprite->source.w, sprite->source.h);
                }
                bitmap_free(&sprite->bitmap);
                sprite->bitmap = page->bitmap;
                // now packed_rect is rotated, so do not copy the original rect dimensions
                sprite->source = packed_rect;
                sprite->uv = calc_uv(packed_rect, fw, fh, sprite->flags & SPRITE_FLAG_ROTATED);

                page->sprites[page->sprites_num++] = *sprite;
            }
            if (++current_page >= max_pages) {
                log_error("max pages limit %u", max_pages);
                return 0;
            }
        }
    }

//    LOG_PERF("Packed %lu to %lu pages for %lf ms",
//             sprites.size(),
//             pages.size(),
//             get_elapsed_time(timer));

    return current_page;
}

void packAndSaveMultiThreaded(atlas_res_t* resolutions, uint32_t resolutions_num, const char* atlas_name,
                              const char* output_path) {
    pthread_t threads[8];
    pack_thread_args thread_args[8];
    int rc;

    for (uint32_t i = 0; i < resolutions_num; ++i) {
        thread_args[i] = (pack_thread_args) {
                .atlas_name = atlas_name,
                .output_path = output_path,
                .atlas_res = resolutions + i,
        };
        rc = pthread_create(&threads[i], NULL, packAtlasThread, (void*) &thread_args[i]);
        if (rc) {
            log_error("pthread create error: %i", rc);
        }
    }

    for (uint32_t i = 0; i < resolutions_num; ++i) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            log_error("pthread join error: %i", rc);
        }
    }

    log_info("'%s' atlas build completed", atlas_name);
}
