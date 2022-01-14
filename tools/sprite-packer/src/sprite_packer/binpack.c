#include "binpack.h"

/**
 * Based on [RectangleBinPack](https://github.com/juj/RectangleBinPack) library
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    BINPACK_MAX_SCORE = 0x7FFFFFFFu,
    BINPACK_START_SIZE = 32u,
};

/** internal types **/

typedef uint32_t binpack_area_t;
typedef binpack_area_t binpack_score_t;

typedef struct binpack_estimation_t {
    binpack_score_t score1;
    binpack_score_t score2;
} binpack_estimation_t;

typedef struct binpack_placement2_t {
    binpack_dim_t x;
    binpack_dim_t y;
    binpack_dim_t w;
    binpack_dim_t h;
    bool rotated;
} binpack_placement2_t;

typedef bool (binpack_method_t)(binpack_estimation_t* best,
                                const binpack_t* ctx,
                                binpack_rect_t free_rect,
                                binpack_size_t size);

/** Utilities for rects **/

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifndef NDEBUG

static void validate_rect(binpack_rect_t rc) {
    EK_ASSERT(rc.x <= rc.r && rc.y <= rc.b);
    EK_ASSERT(!(rc.x & 0x8000u));
    EK_ASSERT(!(rc.y & 0x8000u));
    EK_ASSERT(!(rc.r & 0x8000u));
    EK_ASSERT(!(rc.b & 0x8000u));
}

static void validate_size(binpack_size_t sz) {
    EK_ASSERT(!(sz.w & 0x8000u));
    EK_ASSERT(!(sz.h & 0x8000u));
}

#define VALIDATE_RECT(rc) validate_rect(rc)
#define VALIDATE_SIZE(sz) validate_size(sz)

#else

#define VALIDATE_RECT(ignored) ((void)(ignored))
#define VALIDATE_SIZE(ignored) ((void)(ignored))

#endif

static binpack_dim_t rect_w(binpack_rect_t rc) {
    VALIDATE_RECT(rc);
    return rc.x1 - rc.x0;
}

static binpack_dim_t rect_h(binpack_rect_t rc) {
    VALIDATE_RECT(rc);
    return rc.y1 - rc.y0;
}

static binpack_size_t binpack_rect_size(binpack_rect_t rc) {
    VALIDATE_RECT(rc);
    return (binpack_size_t) {rect_w(rc), rect_h(rc)};
}

// precondition if 2 rectangles possible to be intersected
static bool test_separated_axis(const binpack_rect_t a, const binpack_rect_t b) {
    VALIDATE_RECT(a);
    VALIDATE_RECT(b);
    return a.x < b.r && a.r > b.x && a.y < b.b && a.b > b.y;
}

static bool contains(const binpack_rect_t a, const binpack_rect_t b) {
    VALIDATE_RECT(a);
    VALIDATE_RECT(b);
    return a.x <= b.x && a.y <= b.y && a.r >= b.r && a.b >= b.b;
}

/** node lists **/
static void push_free_node(binpack_rect_t* free, uint32_t* psize, const binpack_rect_t rect) {
    VALIDATE_RECT(rect);
    uint32_t end = *psize;
    for (uint32_t i = 0; i < end; ++i) {
        const binpack_rect_t free_rect = free[i];
        if (test_separated_axis(rect, free_rect)) {
            if (contains(free_rect, rect)) {
                return;
            }
            if (contains(rect, free_rect)) {
                uint32_t back = (*psize) - 1;
                if (back != i) {
                    free[i] = free[back];
                }
                *psize = back;
                --end;
                --i;
            }
        }
    }
    free[(*psize)++] = rect;
}

static void split_free_node(binpack_rect_t* out_vec, uint32_t* out_size, const binpack_rect_t free_node,
                            const binpack_rect_t used) {
    VALIDATE_RECT(free_node);
    VALIDATE_RECT(used);
    // Test with SAT if the rectangles even intersect.
    if (!test_separated_axis(used, free_node)) {
        push_free_node(out_vec, out_size, free_node);
        return;
    }

    if (used.x < free_node.r && used.r > free_node.x) {
        // New node at the top side of the used node.
        if (used.y > free_node.y && used.y < free_node.b) {
            binpack_rect_t new_node = free_node;
            new_node.b = used.y;
            push_free_node(out_vec, out_size, new_node);
        }

        // New node at the bottom side of the used node.
        if (used.b < free_node.b) {
            binpack_rect_t new_node = free_node;
            new_node.y = used.b;
            new_node.b = free_node.b;
            push_free_node(out_vec, out_size, new_node);
        }
    }

    if (used.y < free_node.b && used.b > free_node.y) {
        // New node at the left side of the used node.
        if (used.x > free_node.x && used.x < free_node.r) {
            binpack_rect_t new_node = free_node;
            new_node.r = used.x;
            push_free_node(out_vec, out_size, new_node);
        }

        // New node at the right side of the used node.
        if (used.r < free_node.r) {
            binpack_rect_t new_node = free_node;
            new_node.x = used.r;
            new_node.r = free_node.r;
            push_free_node(out_vec, out_size, new_node);
        }
    }
}

static bool binpack_choose(binpack_t* context,
                           binpack_method_t method,
                           binpack_estimation_t* best,
                           binpack_placement2_t* placement,
                           binpack_size_t size) {
    VALIDATE_SIZE(size);
    const bool allow_rotation = (context->pack_flags & BINPACK_DISABLE_ROTATION) == 0;
    bool rotated = false;
    const uint32_t free_rects_count = context->free_nodes_count;
    uint32_t best_free_rect = free_rects_count;
    for (uint32_t i = 0; i < free_rects_count; ++i) {
        const binpack_rect_t free_node = context->free_nodes[i];
        const binpack_size_t free_size = binpack_rect_size(free_node);
        if (free_size.w >= size.w && free_size.h >= size.h && method(best, context, free_node, size)) {
            best_free_rect = i;
        }
    }
    if (allow_rotation) {
        const binpack_size_t rot_size = {size.h, size.w};
        for (uint32_t i = 0; i < free_rects_count; ++i) {
            const binpack_rect_t free_node = context->free_nodes[i];
            const binpack_size_t free_size = binpack_rect_size(free_node);
            if (free_size.w >= rot_size.w && free_size.h >= rot_size.h && method(best, context, free_node, rot_size)) {
                best_free_rect = i;
                rotated = true;
            }
        }
    }
    if (best_free_rect != free_rects_count) {
        const binpack_rect_t free_rect = context->free_nodes[best_free_rect];
        placement->x = free_rect.x0;
        placement->y = free_rect.y0;
        placement->rotated = rotated;
        if (!rotated) {
            placement->w = size.w;
            placement->h = size.h;
        } else {
            placement->w = size.h;
            placement->h = size.w;
        }
        VALIDATE_SIZE(*placement);
        return true;
    }
    return false;
}

/// Places the given rectangle into the bin.
static void binpack_place(binpack_t* context, const binpack_rect_t node) {
    binpack_rect_t next_free_nodes[BINPACK_NODES_MAX_COUNT];
    uint32_t next_free_nodes_count = 0;

    for (uint32_t i = 0; i < context->free_nodes_count; ++i) {
        binpack_rect_t free_node = context->free_nodes[i];
        split_free_node(next_free_nodes, &next_free_nodes_count, free_node, node);
    }
    memcpy(context->free_nodes, next_free_nodes, next_free_nodes_count * sizeof(binpack_rect_t));
    context->free_nodes_count = next_free_nodes_count;
    context->used_nodes[context->used_nodes_count++] = node;
}

static bool binpack_bottom_left(binpack_estimation_t* best,
                                const binpack_t* ctx,
                                binpack_rect_t free_rect,
                                binpack_size_t size) {
    UNUSED(ctx);
    const binpack_score_t top_side = (binpack_score_t) size.h + free_rect.y;
    if (top_side < best->score1 || (top_side == best->score1 && free_rect.x < best->score2)) {
        best->score1 = top_side;
        best->score2 = free_rect.x;
        return true;
    }
    return false;
}

static bool
binpack_best_short_side_fit(binpack_estimation_t* best,
                            const binpack_t* ctx,
                            binpack_rect_t free_rect,
                            binpack_size_t size) {
    UNUSED(ctx);
    const binpack_size_t leftover = {
            (binpack_dim_t) (free_rect.r - free_rect.x - size.w),
            (binpack_dim_t) (free_rect.b - free_rect.y - size.h),
    };
    VALIDATE_SIZE(leftover);
    const binpack_dim_t short_side_fit = MIN(leftover.w, leftover.h);
    const binpack_dim_t long_side_fit = MAX(leftover.w, leftover.h);

    if (short_side_fit < best->score1 ||
        (short_side_fit == best->score1 && long_side_fit < best->score2)) {
        best->score1 = short_side_fit;
        best->score2 = long_side_fit;
        return true;
    }
    return false;
}

static bool binpack_best_long_side_fit(binpack_estimation_t* best,
                                       const binpack_t* ctx,
                                       binpack_rect_t free_rect,
                                       binpack_size_t size) {
    UNUSED(ctx);
    const binpack_size_t leftover = {
            (binpack_dim_t) (free_rect.r - free_rect.x - size.w),
            (binpack_dim_t) (free_rect.b - free_rect.y - size.h),
    };
    VALIDATE_SIZE(leftover);
    const binpack_dim_t short_side_fit = MIN(leftover.w, leftover.h);
    const binpack_dim_t long_side_fit = MAX(leftover.w, leftover.h);

    if (long_side_fit < best->score2 ||
        (long_side_fit == best->score2 && short_side_fit < best->score1)) {
        best->score1 = short_side_fit;
        best->score2 = long_side_fit;
        return true;
    }
    return false;
}

static bool binpack_best_area_fit(binpack_estimation_t* best,
                                  const binpack_t* ctx,
                                  binpack_rect_t free_rect,
                                  binpack_size_t size) {
    UNUSED(ctx);
    // opt: don't ABS: free rect is already bigger than {width,height}
    const binpack_dim_t free_w = free_rect.r - free_rect.x;
    const binpack_dim_t free_h = free_rect.b - free_rect.y;
    const binpack_dim_t space_x = free_w - size.w;
    EK_ASSERT(!(space_x & 0x8000u));
    const binpack_dim_t space_y = free_h - size.h;
    EK_ASSERT(!(space_y & 0x8000u));
    const binpack_dim_t short_side_fit = MIN(space_x, space_y);
    const binpack_score_t area_fit = (binpack_score_t) free_w * free_h - (binpack_score_t) size.w * size.h;

    if (area_fit < best->score2 || (area_fit == best->score2 && short_side_fit < best->score1)) {
        best->score1 = short_side_fit;
        best->score2 = area_fit;
        return true;
    }
    return false;
}

/** contact point method utilities **/
/// Returns 0 if the two intervals i1 and i2 are disjoint, or the length of their overlap otherwise.
static binpack_dim_t common_interval_length(
        binpack_dim_t first_begin,
        binpack_dim_t first_end,
        binpack_dim_t second_begin,
        binpack_dim_t second_end) {
    if (first_end < second_begin || second_end < first_begin) {
        return 0;
    }
    EK_ASSERT(MIN(first_end, second_end) >= MAX(first_begin, second_begin));
    return MIN(first_end, second_end) - MAX(first_begin, second_begin);
}

static binpack_score_t contact_point_score_node(const binpack_t* context, const binpack_rect_t rect) {
    VALIDATE_RECT(rect);
    binpack_score_t score = BINPACK_MAX_SCORE;
    const binpack_rect_t target = context->target;
    if (rect.x == target.x || rect.r == target.r) {
        // penalty by height
        score -= rect.b - rect.y;
        EK_ASSERT(!(score & 0x80000000u));
    }

    if (rect.y == target.y || rect.b == target.b) {
        // penalty by width
        score -= rect.r - rect.x;
        EK_ASSERT(!(score & 0x80000000u));
    }

    for (uint32_t i = 0; i < context->used_nodes_count; ++i) {
        const binpack_rect_t used_node = context->used_nodes[i];
        if (used_node.x == rect.r || used_node.r == rect.x) {
            score -= common_interval_length(used_node.y, used_node.b, rect.y, rect.b);
            EK_ASSERT(!(score & 0x80000000u));
        }
        if (used_node.y == rect.b || used_node.b == rect.y) {
            score -= common_interval_length(used_node.x, used_node.r, rect.x, rect.r);
            EK_ASSERT(!(score & 0x80000000u));
        }
    }
    EK_ASSERT(!(score & 0x80000000u));
    return score;
}

static bool binpack_contact_point(binpack_estimation_t* best, const binpack_t* ctx, binpack_rect_t free_rect,
                                  binpack_size_t size) {
    const binpack_rect_t rc = {{free_rect.x,
                                free_rect.y,
                                (binpack_dim_t) (free_rect.x + size.w),
                                (binpack_dim_t) (free_rect.y + size.h)}};
    const binpack_score_t score = contact_point_score_node(ctx, rc);
    if (score < best->score1) {
        best->score1 = score;
        return true;
    }
    return false;
}

static binpack_size_t next_size_power_of_two(binpack_size_t size) {
    return (binpack_size_t) {(binpack_dim_t) (size.h << 1u), size.w};
}

//static binpack_size_t next_size_step(binpack_size_t size) {
//    return (binpack_size_t) {(binpack_dim_t) (size.h + 16u), size.w};
//}

static bool is_size_limit(const binpack_size_t size, const binpack_size_t max_size) {
    return size.h >= max_size.h && size.w >= max_size.w;
}

static uint32_t get_area(const binpack_rect_t* rects, uint32_t count) {
    binpack_area_t area = 0u;
    for (uint32_t i = 0; i < count; ++i) {
        const binpack_rect_t rc = rects[i];
        area += (rc.r - rc.x) * (rc.b - rc.y);
    }
    return area;
}

static binpack_size_t estimate_start_size(const binpack_area_t area, const binpack_size_t max_size) {
    binpack_size_t size = {BINPACK_START_SIZE, BINPACK_START_SIZE};
    while (area > size.w * size.h && !is_size_limit(size, max_size)) {
        size = next_size_power_of_two(size);
    }
    return size;
}

static void binpack_resize(binpack_t* context, binpack_size_t size) {
    VALIDATE_SIZE(size);
    context->target = (binpack_rect_t) {{0u, 0u, size.w, size.h}};
}

static void binpack_reset_nodes(binpack_t* context) {
    context->free_nodes[0] = context->target;
    context->free_nodes_count = 1;
    context->used_nodes_count = 0;
}

// input sizes
// output: rects & indices to original
static bool binpack_run_method(binpack_t* ctx, binpack_method_t method) {
    binpack_reset_nodes(ctx);
    memset(ctx->flags, 0, ctx->rects_num);

    const uint32_t total = ctx->rects_num;
    for (uint32_t j = 0; j < total; ++j) {
        binpack_estimation_t best = {BINPACK_MAX_SCORE, BINPACK_MAX_SCORE};
        binpack_placement2_t best_place = {0};
        uint32_t best_rect_index = total;
        for (uint32_t i = 0; i < total; ++i) {
            if (!(ctx->flags[i] & BINPACK_PACKED)) {
                const binpack_size_t size = binpack_rect_size(ctx->rects[i]);
                if (binpack_choose(ctx, method, &best, &best_place, size)) {
                    best_rect_index = i;
                }
            }
        }

        if (best_rect_index == total) {
            return false;
        }

        binpack_place(ctx, (binpack_rect_t) {{
                                                     best_place.x,
                                                     best_place.y,
                                                     best_place.x + best_place.w,
                                                     best_place.y + best_place.h
                                             }});

        uint8_t flags = BINPACK_PACKED;
        if (best_place.rotated) {
            flags |= BINPACK_ROTATED;
        }
        ctx->flags[best_rect_index] |= flags;

        binpack_rect_t rc = ctx->rects[best_rect_index];
        // tricky translate rect to 0,0 and translate back to `best_place` position to save original w/h,
        // because best place could have rotated size for real node placing
        rc.x1 += best_place.x - rc.x0;
        rc.y1 += best_place.y - rc.y0;
        rc.x0 = best_place.x;
        rc.y0 = best_place.y;
        ctx->rects[best_rect_index] = rc;
    }

    return true;
}

void binpack_init(binpack_t* context, binpack_dim_t max_width, binpack_dim_t max_height) {
    context->max_size.w = max_width;
    context->max_size.h = max_height;
    context->rects_num = 0;
    context->free_nodes_count = 0;
    context->used_nodes_count = 0;
    context->target = (binpack_rect_t) {0};
    context->canvas = (binpack_size_t) {0};
}

bool binpack_try_size(binpack_t* context) {
    binpack_resize(context, context->canvas);
    if (binpack_run_method(context, binpack_best_area_fit)) {
        return true;
    }
    if (binpack_run_method(context, binpack_contact_point)) {
        return true;
    }
    if (binpack_run_method(context, binpack_bottom_left)) {
        return true;
    }
    if (binpack_run_method(context, binpack_best_long_side_fit)) {
        return true;
    }
    if (binpack_run_method(context, binpack_best_short_side_fit)) {
        return true;
    }
    return false;
}

bool binpack_add(binpack_t* state, int width, int height, int padding, void* user_data_) {
    // filter empty input as well
    if (width > 0 && height > 0) {
        const binpack_dim_t w = (binpack_dim_t) MAX(0, width + padding * 2);
        const binpack_dim_t h = (binpack_dim_t) MAX(0, height + padding * 2);
        // filter ultra-big entries
        if (w <= state->max_size.w && h <= state->max_size.h) {
            uint32_t n = state->rects_num++;
            state->rects[n] = (binpack_rect_t) {.x1 = w, .y1 = h};
            state->user_data[n] = user_data_;
            state->flags[n] = 0;
            return true;
        }
    }
    return false;
}

bool binpack_run(binpack_t* context) {
    context->canvas = estimate_start_size(get_area(context->rects, context->rects_num), context->max_size);

    //LOG_PERF("Start pack |%u| nodes to |%u %u|", nodes.size(), res.width, res.height);
    while (!binpack_try_size(context)) {
        if (is_size_limit(context->canvas, context->max_size)) {
            return false;
        }
        context->canvas = next_size_power_of_two(context->canvas);
        //LOG_PERF("Enlarge to %u %u", res.width, res.height);
    }
    return true;
}

#ifdef __cplusplus
}
#endif
