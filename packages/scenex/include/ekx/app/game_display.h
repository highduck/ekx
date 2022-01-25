#ifndef EKX_APP_DISPLAY_H
#define EKX_APP_DISPLAY_H

#include <ek/gfx.h>
#include <ek/math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct game_display_info {
    // when game drawing inside window viewport
    rect_t destinationViewport;// = {};
    vec2_t size;// = vec2(1,1);
    vec2_t window;// = vec2(1,1);
    vec4_t insets;// = {};
    vec4_t userInsetsAbsolute;// = {};
    vec4_t userInsetsRelative;// = {};
    float dpiScale;// = 1.0f;
} game_display_info;

typedef struct game_display {
    game_display_info info;//{};
    sg_pass pass;
    sg_image color;
    sg_image depthStencil;
    void* screenshotBuffer;
    bool simulated;
    bool colorFirstClearFlag;
} game_display;

void game_display_update(game_display* display);

bool game_display_begin(game_display* display, sg_pass_action* passAction, const char* debugLabel);
void game_display_end(game_display* display);

bool game_display_dev_begin(game_display* display);
void game_display_dev_end(game_display* display);

// implementation enabled only for EK_UITEST
void game_display_screenshot(const game_display* display, const char* filename);

// prints app's global display/window metrics: size and insets
void log_app_display_info();

#ifdef __cplusplus
}
#endif

#endif // EKX_APP_DISPLAY_H
