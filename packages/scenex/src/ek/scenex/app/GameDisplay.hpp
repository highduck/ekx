#pragma once

#include <ek/gfx.h>
#include <ek/math.h>

namespace ek {

struct GameDisplayInfo {
    // when game drawing inside window viewport
    rect_t destinationViewport = {};
    vec2_t size = vec2(1,1);
    vec2_t window = vec2(1,1);
    vec4_t insets = {};
    vec4_t userInsetsAbsolute = {};
    vec4_t userInsetsRelative = {};
    float dpiScale = 1.0f;
};

class GameDisplay {
public:

    GameDisplayInfo info{};

    sg_pass pass{};
    sg_image color = {0};
    sg_image depthStencil = {0};
    void* screenshotBuffer = nullptr;
    bool simulated = false;
    bool colorFirstClearFlag = false;

    void update();

    bool beginGame(sg_pass_action& passAction, const char* debugLabel = "Game");
    void endGame();

    bool beginOverlayDev();
    void endOverlayDev();

    // implementation enabled only for EK_UITEST
    void screenshot(const char* filename) const;
};

}
