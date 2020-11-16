#include "demo_main.hpp"
#include "sample_text.hpp"
#include "sample_flash.hpp"
#include "3d/sample_3d.hpp"
#include "piko/sample_piko.hpp"
#include "sample_integrations.hpp"

#include <ek/math/rand.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/scenex/scene_system.hpp>
#include <ek/scenex/asset2/builtin_assets.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/components/interactive.hpp>
#include <ek/scenex/components/layout.hpp>
#include <ui/minimal.hpp>

namespace ek {

std::vector<std::function<SampleBase*()>> sampleFactory;
int currentSampleIndex = 0;
std::unique_ptr<SampleBase> currentSample = nullptr;
ecs::entity tfSampleTitle;

void setCurrentSample(int index) {
    int samplesCount = static_cast<int>(sampleFactory.size());
    currentSampleIndex = index;
    if (currentSampleIndex >= samplesCount) {
        currentSampleIndex = 0;
    }
    if (currentSampleIndex < 0) {
        currentSampleIndex = samplesCount - 1;
    }
    currentSample.reset(sampleFactory[currentSampleIndex]());
    get_drawable<Text2D>(tfSampleTitle).text = currentSample->title;
}

void scrollSample(int delta) {
    setCurrentSample(currentSampleIndex + delta);
}

void main() {
    app::g_app.window_cfg = {
            "ekx",
            app::vec2{360, 480},
            true};
    app::g_app.on_device_ready << [] {
        run_app<DemoApp>();
    };
    start_application();
}
}

using namespace ek;

DemoApp::DemoApp() :
        basic_application() {
}

void DemoApp::initialize() {
    basic_application::initialize();
}

void DemoApp::preload() {
    basic_application::preload();

    clear_color_enabled = true;

    SampleText::prepareInternalResources();
}

void DemoApp::update_frame(float dt) {
    basic_application::update_frame(dt);

    scene_pre_update(root, dt);

    if (currentSample) {
        currentSample->update(dt);
    }

    //update_game_title_start(w);
    scene_post_update(root, dt);
}

void DemoApp::render_frame() {
//    base_app_type::render_frame();
    if (!started_) return;

    if (currentSample) {
        currentSample->draw();
    }

    scene_render(root);
}

void DemoApp::start_game() {
//    setup_game(w, game);

    SampleBase::samplesContainer = create_node_2d("sample");
    append(game, SampleBase::samplesContainer);

    auto prev = createButton("<", [] { scrollSample(-1); });
    auto next = createButton(">", [] { scrollSample(+1); });
    ecs::assign<layout_t>(prev).enableAlignX(0, 30);
    ecs::assign<layout_t>(next).enableAlignX(1, -30);

    tfSampleTitle = create_node_2d("title");
    addText(tfSampleTitle, "");
    ecs::assign<layout_t>(tfSampleTitle).enableAlignX(0.5);

    auto controls = create_node_2d("controls");
    append(controls, tfSampleTitle);
    append(controls, prev);
    append(controls, next);
    ecs::assign<layout_t>(controls).enableAlignY(1, -30);

    append(game, controls);

    sampleFactory.emplace_back([] { return new SampleText(); });
    sampleFactory.emplace_back([] { return new Sample3D(); });
    sampleFactory.emplace_back([] { return new SamplePiko(); });
    sampleFactory.emplace_back([] { return new SampleFlash("test1"); });
    sampleFactory.emplace_back([] { return new SampleFlash("test2"); });
    sampleFactory.emplace_back([] { return new SampleIntegrations(); });
    setCurrentSample(0);
}

DemoApp::~DemoApp() = default;
