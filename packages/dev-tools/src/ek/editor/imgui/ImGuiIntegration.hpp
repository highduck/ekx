#pragma once

#include <ek/gfx.h>
#include <ek/app.h>
#include <ek/math/Vec.hpp>
#include <ek/util/Res.hpp>
#include <ek/ds/String.hpp>

#include <util/sokol_gfx_imgui.h>

struct ImDrawData;

namespace ek {

class ImGuiIntegration final {
public:
    ImGuiIntegration();

    ~ImGuiIntegration();

    void on_event(const ek_app_event& event);

    void on_frame_completed();

    void begin_frame(float dt);

    void end_frame();

    sg_imgui_t sokol_gfx_gui_state{};

private:

    void setup();

    void init_font_image();

private:
    String clipboard_text_{};
    sg_image font_image;
    float dpiScale = 2.0f;
};

}

