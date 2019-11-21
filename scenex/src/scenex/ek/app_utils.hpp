#pragma once

#include <ek/utility/static_history_buffer.hpp>
#include <ek/timer.hpp>

#include <string>

namespace scenex {

void create_builtin();

class frame_stats_t {
public:
    void update();

    void draw();

private:
    ek::fps_meter_t fps_meter_{};
    ek::static_history_buffer<float, uint8_t, 128> fps_history_{};
    uint32_t draw_calls_ = 0u;
    uint32_t triangles_ = 0u;
};

}

