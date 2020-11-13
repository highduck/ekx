#include "profiler.hpp"
#include "basic_application.hpp"

#include <memory>
#include <unordered_map>
#include <ek/util/static_history_buffer.hpp>
#include <ek/util/timer.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/app/device.hpp>
#include <ek/scenex/text/font.hpp>
#include <ek/scenex/text/text_drawer.hpp>

#ifndef NDEBUG
#define ENABLE_PROFILER
#endif

namespace ek {

using namespace std;

struct RealTimeGraph {
    const char* name = "";
    const char* titleFormat = "%0.0f";
    const char* markerFormat = "%0.0f";
    float thMin = 1.0f / 60.0f;
    float thMax = 1.0f / 30.0f;
    float value{};
    static_history_buffer<float, 60> history{};

    void draw() const {
        float width = history.size();
        static char buf[256];
        static TextDrawer textDrawer;
        textDrawer.format.font.setID("Cousine-Regular");
        textDrawer.format.size = 10.0f;
        textDrawer.format.leading = 1.0f;

        const int samples = history.capacity();
        float x = 0.0f;
        float H = 30;
        float prev = round(H * (history.at(0) - min) / (max - min));
        draw2d::state.set_empty_texture();
        draw2d::quad(0, 0, width, H, 0x77000000_argb);
        for (int i = 0; i < samples; ++i) {
            float val = history.at(i);
            float curr = round(H * (val - min) / (max - min));
            draw2d::quad(x, H - curr, 1, curr - prev + 1,
                         lerp(0x00FF00_rgb, 0xFF0000_rgb, math::clamp((val - thMin) / (thMax - thMin), 0.0f, 1.0f)));
            prev = curr;
            x += 1;
        }
        snprintf(buf, sizeof(buf) - 1, markerFormat, history.at_backward(0));
        textDrawer.rect.position = {width, H - prev + 0.5f * textDrawer.format.size};
        textDrawer.draw(buf);

        snprintf(buf, sizeof(buf) - 1, titleFormat, value);
        textDrawer.rect.position = {2.0f, 2.0f + textDrawer.format.size};
        textDrawer.draw(buf);
    }

    float min = 0.0f;
    float max = 33.3f;
};

class Profiler::Impl {
public:
    Impl() = default;

    ~Impl() = default;

    fps_meter_t fpsMeter{};
    RealTimeGraph graphFrameTime;
    RealTimeGraph graphTriangles;
    RealTimeGraph graphDrawCalls;
    RealTimeGraph graphGameTime;
};

void Profiler::update() {
#ifdef ENABLE_PROFILER
    auto& app = resolve<basic_application>();
    auto dt = app.frame_timer.delta_time();
    impl->fpsMeter.update(dt);
    impl->graphFrameTime.history.write(dt * 1000.0f);
    impl->graphFrameTime.value = impl->fpsMeter.fps();
    impl->graphDrawCalls.history.write(draw2d::get_stat_draw_calls());
    impl->graphDrawCalls.value = draw2d::get_stat_draw_calls();
    impl->graphTriangles.history.write(draw2d::get_stat_triangles());
    impl->graphTriangles.value = draw2d::get_stat_triangles();
#endif
}

void Profiler::draw() {
#ifdef ENABLE_PROFILER
    const auto scale = static_cast<float>(app::g_app.content_scale);
    auto insets = get_screen_insets();
    draw2d::state.save_matrix()
            .translate(insets[0], insets[1])
            .scale(scale, scale);
    draw2d::state.set_empty_texture();

    impl->graphFrameTime.draw();
    draw2d::state.translate(0, 50);
    impl->graphDrawCalls.draw();
    draw2d::state.translate(0, 50);
    impl->graphTriangles.draw();
    draw2d::state.translate(0, 50);
    impl->graphGameTime.draw();

    draw2d::state.restore_matrix();
#endif
}

Profiler::Profiler() {
#ifdef ENABLE_PROFILER
    impl = std::make_unique<Impl>();
    impl->graphFrameTime.name = "FPS";
    impl->graphFrameTime.titleFormat = "FPS: %0.0f";
    impl->graphFrameTime.markerFormat = "%0.2f ms";
    impl->graphFrameTime.thMin = 1000.0f / 60.0f;
    impl->graphFrameTime.thMax = 1000.0f / 30.0f;
    impl->graphFrameTime.min = 16.0f;
    impl->graphFrameTime.max = 100.0f;

    impl->graphTriangles.titleFormat = "TRIS: %0.0f";
    impl->graphTriangles.markerFormat = "";
    impl->graphTriangles.thMin = 1000;
    impl->graphTriangles.thMax = 2000;
    impl->graphTriangles.min = 0;
    impl->graphTriangles.max = 3000;
    impl->graphTriangles.markerFormat = "<";

    impl->graphDrawCalls.titleFormat = "DC: %0.0f";
    impl->graphDrawCalls.markerFormat = "";
    impl->graphDrawCalls.thMin = 20;
    impl->graphDrawCalls.thMax = 50;
    impl->graphDrawCalls.min = 0;
    impl->graphDrawCalls.max = 200;

    impl->graphGameTime.titleFormat = "FRAME: %0.2f ms";
    impl->graphGameTime.markerFormat = "";
    impl->graphGameTime.thMin = 4;
    impl->graphGameTime.thMax = 8;
    impl->graphGameTime.min = 0;
    impl->graphGameTime.max = 16;
#endif
}

Profiler::~Profiler() = default;

void Profiler::setGameTime(float time) {
#ifdef ENABLE_PROFILER
    impl->graphGameTime.history.write(time);
    impl->graphGameTime.value = time;
#endif
}

}