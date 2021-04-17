#pragma once

#include <ek/graphics/graphics.hpp>
#include <ek/math/vec.hpp>

namespace ek {

struct GameDisplayInfo {
    float2 size{1.0f, 1.0f};
    float2 window{1.0f, 1.0f};
    float4 insets{0.0f, 0.0f, 0.0f, 0.0f};
    float dpiScale = 1.0f;
};

class GameDisplay {
public:

    GameDisplayInfo info{};

    sg_pass pass{};
    graphics::Texture* color = nullptr;
    graphics::Texture* depthStencil = nullptr;
    void* screenshotBuffer = nullptr;
    bool simulated = false;

    void update();

    bool beginGame(const sg_pass_action& passAction);
    void endGame();

    bool beginOverlayDev();
    void endOverlayDev();

    // implementation enabled only for EK_UITEST
    void screenshot(const char* filename) const;
};

}
