#pragma once

#include "../GameDisplay.hpp"

#include <ek/draw2d/drawer.hpp>
#include <ek/app/device.hpp>
#include <ek/app/app.hpp>

#ifdef EK_UITEST
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#endif

namespace ek {

bool GameDisplay::beginGame(sg_pass_action& passAction, const char* debugLabel) {
    const auto w = static_cast<int>(info.size.x);
    const auto h = static_cast<int>(info.size.y);
    if (w <= 0 || h <= 0) {
        return false;
    }
    sg_push_debug_group(debugLabel);

    if (simulated) {
        draw2d::state.framebufferColor = color;
        draw2d::state.framebufferDepthStencil = depthStencil;

        if (colorFirstClearFlag) {
            passAction.colors[0].action = SG_ACTION_CLEAR;
            colorFirstClearFlag = false;
        }
        sg_begin_pass(pass, &passAction);
    } else {
        sg_begin_default_pass(&passAction, w, h);
    }

    return true;
}

void GameDisplay::endGame() {
    if (simulated) {
        sg_end_pass();

        draw2d::state.framebufferColor = nullptr;
        draw2d::state.framebufferDepthStencil = nullptr;
    }
    sg_pop_debug_group();
}

bool GameDisplay::beginOverlayDev() {
    using app::g_app;
    const auto fw = g_app.drawableWidth;
    const auto fh = g_app.drawableHeight;
    const auto w = static_cast<int>(fw);
    const auto h = static_cast<int>(fh);
    if (w <= 0 || h <= 0) {
        return false;
    }

    if (simulated) {

        static sg_pass_action pass_action{};
        pass_action.colors[0].action = SG_ACTION_DONTCARE;

        sg_begin_default_pass(pass_action, w, h);

        const float scale = std::min(fw / info.size.x, fh / info.size.y);
        // draw offscreen

        sg_push_debug_group("Game viewport");
// todo: temp disable
        draw2d::begin({0, 0, fw, fh});
        draw2d::state.setTextureRegion(color, rect_f::zero_one);
        draw2d::quad(0, 0, scale * info.size.x, scale * info.size.y);
        draw2d::end();

        sg_pop_debug_group();
    }

    return true;
}

void GameDisplay::endOverlayDev() {

    sg_end_pass();
    //sg_commit();
//    if (simulated) {
//        screenshot("screenshot.png");
//    }
}

graphics::Texture* createGameDisplayTexture(int w, int h, bool isColor, const char* label) {
    sg_image_desc desc{};
    desc.type = SG_IMAGETYPE_2D;
    desc.render_target = true;
    desc.width = w;
    desc.height = h;
    desc.usage = SG_USAGE_IMMUTABLE;
    if (!isColor) {
        desc.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    }
    desc.min_filter = SG_FILTER_NEAREST;
    desc.mag_filter = SG_FILTER_NEAREST;
    desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    desc.label = label;
    return new graphics::Texture(desc);
}

void GameDisplay::update() {
    if (simulated) {
        // limit min size
        info.size.x = fmax(16.0f, info.size.x);
        info.size.y = fmax(16.0f, info.size.y);

        const auto w = static_cast<int>(info.size.x);
        const auto h = static_cast<int>(info.size.y);
        if (color == nullptr || color->desc.width != w || color->desc.height != h) {
            delete color;
            color = createGameDisplayTexture(w, h, true, "game-display-color");
            colorFirstClearFlag = true;

            if (app::g_app.config.needDepth) {
                delete depthStencil;
                depthStencil = createGameDisplayTexture(w, h, false, "game-display-depth");
            }

            sg_destroy_pass(pass);
            sg_pass_desc passDesc{};
            passDesc.color_attachments[0].image = color->image;
            if (depthStencil) {
                passDesc.depth_stencil_attachment.image = depthStencil->image;
            }
            pass = sg_make_pass(passDesc);

            free(screenshotBuffer);
            screenshotBuffer = malloc(w * h * 4);
        }
    } else {
        using app::g_app;
        info.size.x = g_app.drawableWidth;
        info.size.y = g_app.drawableHeight;
        info.window.x = g_app.windowWidth;
        info.window.y = g_app.windowHeight;
        getScreenInsets(info.insets.data());
        info.dpiScale = g_app.dpiScale;

        info.destinationViewport.position = float2::zero;
        info.destinationViewport.size.x = g_app.drawableWidth;
        info.destinationViewport.size.y = g_app.drawableHeight;
    }
}

void GameDisplay::screenshot(const char* filename) const {
#ifdef EK_UITEST
    if (simulated && screenshotBuffer) {
        const auto wi = color->desc.width;
        const auto he = color->desc.height;
        if (color->getPixels(screenshotBuffer)) {
            stbi_write_png(filename, wi, he, 4, screenshotBuffer, 4 * wi);
        }
    }
#endif
}

}
