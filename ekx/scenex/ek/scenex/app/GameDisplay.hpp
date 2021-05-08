#pragma once

#include <ek/graphics/graphics.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>

namespace ek {

struct GameDisplayInfo {
    // when game drawing inside window viewport
    rect_f destinationViewport{};
    float2 size{1.0f, 1.0f};
    float2 window{1.0f, 1.0f};
    float4 insets{0.0f, 0.0f, 0.0f, 0.0f};
    float dpiScale = 1.0f;
    float4 userInsetsAbsolute = float4::zero;
    float4 userInsetsRelative = float4::zero;
};

class GameDisplay {
public:

    GameDisplayInfo info{};

    sg_pass pass{};
    graphics::Texture* color = nullptr;
    graphics::Texture* depthStencil = nullptr;
    void* screenshotBuffer = nullptr;
    bool simulated = false;
    bool colorFirstClearFlag = false;

    void update();

    bool beginGame(sg_pass_action& passAction);
    void endGame();

    bool beginOverlayDev();
    void endOverlayDev();

    // implementation enabled only for EK_UITEST
    void screenshot(const char* filename) const;
};

}
