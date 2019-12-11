#pragma once

#include <ek/util/static_history_buffer.hpp>
#include <ek/util/timer.hpp>

#include <string>

namespace ek {

void create_builtin();

class frame_stats_t {
public:
    void update();

    void draw();

private:
    fps_meter_t fps_meter_{};
    static_history_buffer<float, uint8_t, 128> fps_history_{};
    uint32_t draw_calls_ = 0u;
    uint32_t triangles_ = 0u;
};

}

