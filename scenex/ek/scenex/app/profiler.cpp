#include "profiler.hpp"

#include <memory>
#include <unordered_map>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/ds/StaticHistoryBuffer.hpp>
#include <ek/timers.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/app/device.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/scenex/text/TextEngine.hpp>

// TODO: EK_DEV_BUILD
#ifndef ENABLE_PROFILER

#if !defined(NDEBUG) || defined(EK_DEV_TOOLS)
#define ENABLE_PROFILER
#endif

#endif

namespace ek {

using namespace std;

static TextFormat devTextFormat{"Cousine-Regular", 10};

struct RealTimeGraph {
    std::string name{};
    const char *titleFormat = "%s: %0.0f";
    const char *markerFormat = "%0.0f";
    float thMin = 1.0f / 60.0f;
    float thMax = 1.0f / 30.0f;
    float accum{};
    float value{};
    StaticHistoryBuffer<float, 60> history{};

    float height = 30.0f;

    [[nodiscard]] float calculateY(float val) const {
        return round(height * (val - min) / (max - min));
    }

    [[nodiscard]] argb32_t calculateColor(float val) const {
        return lerp(0x00FF00_rgb, 0xFF0000_rgb,
                    math::clamp((val - thMin) / (thMax - thMin), 0.0f, 1.0f));
    }

    void drawGraph() {
        float width = history.size();

        const int samples = history.capacity();
        float x = 0.0f;
        float x0 = 0.0f;
        float prev = calculateY(history.at(0));
        draw2d::state.set_empty_texture();
        draw2d::quad(0, 0, width, height, 0x77000000_argb);
        for (int i = 0; i < samples; ++i) {
            float val = history.at(i);
            float curr = calculateY(val);
            if (curr != prev || i + 1 == samples) {
                if (x > x0) {
                    draw2d::quad(x0, height - prev, x - x0, 1, calculateColor(prev));
                }
                if (curr != prev) {
                    draw2d::quad(x, height - curr, 1, curr - prev, calculateColor(curr));
                }
                x0 = x + 1;
            }
            prev = curr;
            x += 1;
        }
    }

    void drawText() const {
        devTextFormat.leading = 1;

//        float width = history.size();
//        float lastValue = history.at_backward(0);
//        float y = calculateY(lastValue);
//        TextDrawer::shared.position = {width, height - y + 0.5f * textFormat.size};
//        TextDrawer::shared.drawFormat(markerFormat, history.at_backward(0));

        TextEngine::shared.format = devTextFormat;
        TextEngine::shared.position = {2.0f, 2.0f + devTextFormat.size};
        TextEngine::shared.drawFormat(titleFormat, name.c_str(), value);
    }

    float min = 0.0f;
    float max = 33.3f;
};

class Profiler::Impl {
public:
    Impl() = default;

    ~Impl() = default;

    FpsMeter fpsMeter{};
    RealTimeGraph graphFrameTime;
    RealTimeGraph graphTriangles;
    RealTimeGraph graphDrawCalls;
    RealTimeGraph graphFillRate;
    unordered_map<string, RealTimeGraph> frameGraphs;
};

void Profiler::update(float dt) {
#ifdef ENABLE_PROFILER
    impl->fpsMeter.update(dt);
    impl->graphFrameTime.history.write(dt * 1000.0f);
    impl->graphFrameTime.value = impl->fpsMeter.getAverageFPS();

    auto stats = draw2d::getDrawStats();
    impl->graphDrawCalls.history.write(stats.drawCalls);
    impl->graphDrawCalls.value = stats.drawCalls;
    impl->graphTriangles.history.write(stats.triangles);
    impl->graphTriangles.value = stats.triangles;

    auto drawableSize = app::g_app.drawable_size;
    auto fillRate = stats.fillArea / (drawableSize.x * drawableSize.y);
    impl->graphFillRate.history.write(fillRate);
    impl->graphFillRate.value = fillRate;
#endif
}

void Profiler::draw() {
#ifdef ENABLE_PROFILER
    const auto scale = static_cast<float>(app::g_app.content_scale);
    auto insets = get_screen_insets();
    draw2d::state.matrix = matrix_2d{scale, 0, 0, scale, insets.x, insets.y};
    draw2d::state.set_empty_texture();

    draw2d::state.save_matrix();
    impl->graphFrameTime.drawGraph();
    draw2d::state.translate(0, 35);
    impl->graphDrawCalls.drawGraph();
    draw2d::state.translate(0, 35);
    impl->graphTriangles.drawGraph();
    draw2d::state.translate(0, 35);
    impl->graphFillRate.drawGraph();

    for (auto& it : impl->frameGraphs) {
        draw2d::state.translate(0, 35);
        it.second.history.write(it.second.accum);
        it.second.drawGraph();
        it.second.value = it.second.accum;
        it.second.accum = 0.0f;
    }
    draw2d::state.restore_matrix();

    draw2d::state.save_matrix();
    impl->graphFrameTime.drawText();
    draw2d::state.translate(0, 35);
    impl->graphDrawCalls.drawText();
    draw2d::state.translate(0, 35);
    impl->graphTriangles.drawText();
    draw2d::state.translate(0, 35);
    impl->graphFillRate.drawText();

    for (auto& it : impl->frameGraphs) {
        draw2d::state.translate(0, 35);
        it.second.drawText();
    }

    draw2d::state.restore_matrix();
#endif
}

Profiler::Profiler() {
#ifdef ENABLE_PROFILER
    impl = std::make_unique<Impl>();
    impl->graphFrameTime.name = "FPS";
    impl->graphFrameTime.titleFormat = "%s: %0.0f";
    impl->graphFrameTime.markerFormat = "%0.2f ms";
    impl->graphFrameTime.thMin = 1000.0f / 60.0f;
    impl->graphFrameTime.thMax = 1000.0f / 30.0f;
    impl->graphFrameTime.min = 16.0f;
    impl->graphFrameTime.max = 100.0f;

    impl->graphTriangles.name = "TRIS";
    impl->graphTriangles.titleFormat = "%s: %0.0f";
    impl->graphTriangles.markerFormat = "";
    impl->graphTriangles.thMin = 1000;
    impl->graphTriangles.thMax = 2000;
    impl->graphTriangles.min = 0;
    impl->graphTriangles.max = 3000;

    impl->graphDrawCalls.name = "DC";
    impl->graphDrawCalls.titleFormat = "%s: %0.0f";
    impl->graphDrawCalls.markerFormat = "";
    impl->graphDrawCalls.thMin = 20;
    impl->graphDrawCalls.thMax = 50;
    impl->graphDrawCalls.min = 0;
    impl->graphDrawCalls.max = 200;

    impl->graphFillRate.name = "FR";
    impl->graphFillRate.titleFormat = "%s: %0.2f";
    impl->graphFillRate.markerFormat = "";
    impl->graphFillRate.thMin = 1;
    impl->graphFillRate.thMax = 3;
    impl->graphFillRate.min = 0;
    impl->graphFillRate.max = 5;
#endif
}

Profiler::~Profiler() = default;

void Profiler::addTime(const char *name, float time) {
    (void)name;
    (void)time;
#ifdef ENABLE_PROFILER
    auto& graphs = impl->frameGraphs;
    {
        const auto& it = graphs.find(name);
        if (it == graphs.end()) {
            RealTimeGraph graph{};
            graph.name = name;
            graph.titleFormat = "%s: %0.2f";
            graph.markerFormat = "";
            graph.thMin = 4;
            graph.thMax = 8;
            graph.min = 0;
            graph.max = 16;
            graphs[name] = std::move(graph);
        }
    }
    auto& graph = graphs[name];
    graph.accum += time;
#endif
}

}