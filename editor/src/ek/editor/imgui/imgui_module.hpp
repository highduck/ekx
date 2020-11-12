#pragma once

#include <ek/graphics/graphics.hpp>
#include <ek/app/app.hpp>
#include <ek/math/vec.hpp>
#include <string>

struct ImDrawData;

namespace ek {

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
    graphics::buffer_t* vertex_buffer_ = nullptr;
    graphics::buffer_t* index_buffer_ = nullptr;
    graphics::texture_t* texture_ = nullptr;
    graphics::program_t* program_ = nullptr;
    std::string clipboard_text_{};
};

}

