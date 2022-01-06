#pragma once

#include "../GameDisplay.hpp"

#include <ek/canvas.h>
#include <ek/app.h>

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
        canvas.framebuffer_color = color;
        canvas.framebuffer_depth = depthStencil;

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

        canvas.framebuffer_color = {0};
        canvas.framebuffer_depth = {0};
    }
    sg_pop_debug_group();
}

bool GameDisplay::beginOverlayDev() {
    const auto fw = ek_app.viewport.width;
    const auto fh = ek_app.viewport.height;
    const auto w = static_cast<int>(fw);
    const auto h = static_cast<int>(fh);
    if (w <= 0 || h <= 0) {
        return false;
    }

    if (simulated) {

        sg_pass_action pass_action{};
        pass_action.colors[0].action = SG_ACTION_DONTCARE;

        sg_begin_default_pass(pass_action, w, h);

        const float scale = std::min(fw / info.size.x, fh / info.size.y);
        // draw offscreen

        sg_push_debug_group("Game viewport");
// todo: temp disable
        canvas_begin(fw, fh);
        canvas_set_image(color);
        canvas_set_image_rect(rect_01());
        canvas_quad(0, 0, scale * info.size.x, scale * info.size.y);
        canvas_end();

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

sg_image createGameDisplayImage(int w, int h, bool isColor, const char* label) {
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
    return sg_make_image(desc);
}

void GameDisplay::update() {
    if (simulated) {
        // limit min size
        info.size.x = fmax(16.0f, info.size.x);
        info.size.y = fmax(16.0f, info.size.y);

        const auto w = static_cast<int>(info.size.x);
        const auto h = static_cast<int>(info.size.y);

        int color_img_width = 0;
        int color_img_height = 0;
        if(color.id) {
            const auto color_image_info = sg_query_image_info(color);
            color_img_width = color_image_info.width;
            color_img_height = color_image_info.height;
        }
        if (color.id == 0 || color_img_width != w || color_img_height != h) {
            sg_destroy_image(color);
            color = createGameDisplayImage(w, h, true, "game-display-color");
            color_img_width = w;
            color_img_height = h;

            colorFirstClearFlag = true;

            if (ek_app.config.need_depth) {
                sg_destroy_image(depthStencil);
                depthStencil = createGameDisplayImage(w, h, false, "game-display-depth");
            }

            sg_destroy_pass(pass);
            sg_pass_desc passDesc{};
            passDesc.color_attachments[0].image = color;
            if (depthStencil.id) {
                passDesc.depth_stencil_attachment.image = depthStencil;
            }
            pass = sg_make_pass(passDesc);

            free(screenshotBuffer);
            screenshotBuffer = malloc(w * h * 4);
        }
    } else {
        info.size.x = ek_app.viewport.width;
        info.size.y = ek_app.viewport.height;
        info.window.x = ek_app.viewport.width / ek_app.viewport.scale;
        info.window.y = ek_app.viewport.height / ek_app.viewport.scale;
        info.insets = *(vec4_t*)ek_app.viewport.insets;
        info.dpiScale = ek_app.viewport.scale;

        info.destinationViewport.position = Vec2f::zero;
        info.destinationViewport.size.x = ek_app.viewport.width;
        info.destinationViewport.size.y = ek_app.viewport.height;
    }
}

void GameDisplay::screenshot(const char* filename) const {
    (void) filename;
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
