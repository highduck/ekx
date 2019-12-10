#pragma once

#include <graphics/gl_def.hpp>
#include <ek/app/app.hpp>
#include <ek/math/vec.hpp>
#include <string>

struct ImDrawData;

namespace ek {

class texture_t;

class program_t;

class buffer_object_t;

class imgui_module_t final {
public:
    imgui_module_t();

    ~imgui_module_t();

    void on_event(const app::event_t& event);

    void on_frame_completed();

    void begin_frame(float dt);

    void end_frame();

private:

    void setup();

    void init_fonts();

    void render_frame_data(ImDrawData* draw_data);

private:
    ek::buffer_object_t* vertex_buffer_ = nullptr;
    ek::buffer_object_t* index_buffer_ = nullptr;
    ek::texture_t* texture_ = nullptr;
    ek::program_t* program_ = nullptr;
    bool enabled_ = false;
    std::string clipboard_text_{};
};

}

