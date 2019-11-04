#pragma once

#include <graphics/gl_def.hpp>
#include <platform/Application.h>
#include <ek/math/vec.hpp>
#include <string>

struct ImDrawData;

namespace ek {

class texture_t;

class program_t;

class buffer_object_t;
}

namespace scenex {

class imgui_module_t : public ek::Application::Listener {
public:
    imgui_module_t();

    ~imgui_module_t() final;

    void onKeyEvent(const ek::KeyEvent& event) override;

    void onMouseEvent(const ek::MouseEvent& event) override;

    void onTouchEvent(const ek::TouchEvent& event) override;

    void onAppEvent(const ek::AppEvent& event) override;

    void onDrawFrame() override;

    void on_text_event(const ek::text_event_t& event) override;

    void on_frame_completed() override;

    void begin_frame(float dt);

    void end_frame();

private:

    void setup();

    void init_program();

    void init_fonts();

    void enable_vertex_attributes();

    void render_frame_data(ImDrawData* draw_data);

    void update_mouse_state();

    void update_mouse_cursor();

public:
    std::string clipboard_text_{};

private:
    ek::buffer_object_t* vertex_buffer_;
    ek::buffer_object_t* index_buffer_;
    ek::texture_t* texture_;
    ek::program_t* program_;

    struct program_locations_t {
        GLint tex = 0;
        GLint proj = 0;
        GLint position = 0;
        GLint uv = 0;
        GLint color = 0;
    };

    program_locations_t locations_;
    bool enabled_ = false;
};

}

