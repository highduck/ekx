#ifndef EK_BINPACK_H
#define EK_BINPACK_H

#include <ek/assert.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum binpack_flag_t {
    BINPACK_PACKED = 1u,
    BINPACK_ROTATED = 2u,
} binpack_flag_t;

typedef uint16_t binpack_dim_t;

typedef struct binpack_size_t {
    binpack_dim_t w;
    binpack_dim_t h;
} binpack_size_t;

typedef union binpack_rect_t {
    struct {
        binpack_dim_t x0;
        binpack_dim_t y0;
        binpack_dim_t x1;
        binpack_dim_t y1;
    };
    struct {
        binpack_dim_t x;
        binpack_dim_t y;
        binpack_dim_t r;
        binpack_dim_t b;
    };
} binpack_rect_t;

enum {
    BINPACK_RECTS_MAX_COUNT = 1024u,
    BINPACK_NODES_MAX_COUNT = 2048u,

    /**
     * By default all rects allowed to be rotated (exchange width and height). Add `BINPACK_DISABLE_ROTATION` to `pack_flags` for disabling rotation feature
     */
    BINPACK_DISABLE_ROTATION = 1u,
};

typedef struct binpack_t {
    binpack_rect_t target;
    binpack_size_t canvas;
    binpack_size_t max_size;
    uint32_t pack_flags;

    // nodes
    uint32_t free_nodes_count;
    binpack_rect_t free_nodes[BINPACK_NODES_MAX_COUNT];
    uint32_t used_nodes_count;
    binpack_rect_t used_nodes[BINPACK_NODES_MAX_COUNT];

    // rects
    uint32_t rects_num;
    binpack_rect_t rects[BINPACK_RECTS_MAX_COUNT];
    uint8_t flags[BINPACK_RECTS_MAX_COUNT];
    void* user_data[BINPACK_RECTS_MAX_COUNT];
} binpack_t;

void binpack_init(binpack_t* context, binpack_dim_t max_width, binpack_dim_t max_height);

bool binpack_add(binpack_t* state, int width, int height, int padding, void* user_data_);

/**
 * Try pack all queued rects to the current `canvas` size
 * @param context
 * @return
 */
bool binpack_try_size(binpack_t* context);

/**
 * Pack rects and auto-grows canvas size
 * @param context
 * @return `true` if all rects are packed into the page, `false` if some rects were not packed into `max_size` page
 */
bool binpack_run(binpack_t* context);

#ifdef __cplusplus
}
#endif

#endif // EK_BINPACK_H
