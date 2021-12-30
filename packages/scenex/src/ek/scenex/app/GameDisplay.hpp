#pragma once

#include <ek/gfx.h>
#include <ek/math/Vec.hpp>
#include <ek/math/Rect.hpp>

namespace ek {

struct GameDisplayInfo {
    // when game drawing inside window viewport
    Rect2f destinationViewport{};
    Vec2f size{1.0f};
    Vec2f window{1.0f};
    Vec4f insets{};
    Vec4f userInsetsAbsolute{};
    Vec4f userInsetsRelative{};
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
