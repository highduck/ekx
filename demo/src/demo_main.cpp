#include "demo_main.hpp"
#include "sample_text.hpp"
#include "sample_flash.hpp"
#include "3d/sample_3d.hpp"
#include "piko/sample_piko.hpp"
#include "sample_integrations.hpp"
#include "sim/components/motion.h"
#include "sim/components/attractor.h"
#include "3d/camera_arcball.hpp"

#include <ek/math/rand.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/scenex/asset2/builtin_assets.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ui/minimal.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/util/logger.hpp>

namespace ek {

std::vector<std::function<SampleBase*()>> sampleFactory;
int currentSampleIndex = 0;
std::unique_ptr<SampleBase> currentSample = nullptr;
ecs::entity tfSampleTitle;

void setCurrentSample(int index);

void initSamples() {
    sampleFactory.emplace_back([] { return new SamplePiko(); });
    //sampleFactory.emplace_back([] { return new SampleFlash("test1"); });
    sampleFactory.emplace_back([] { return new SampleFlash("test2"); });
    sampleFactory.emplace_back([] { return new Sample3D(); });
    sampleFactory.emplace_back([] { return new SampleIntegrations(); });
    sampleFactory.emplace_back([] { return new SampleText(); });
    setCurrentSample(0);
}

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
    run_app<DemoApp>({"ekx", app::vec2{360, 480}, true});
}

}

using namespace ek;

DemoApp::DemoApp() :
        basic_application() {
}

void DemoApp::initialize() {
    basic_application::initialize();

    using ecs::the_world;
    the_world.registerComponent<motion_t>();
    the_world.registerComponent<attractor_t>();
    the_world.registerComponent<camera_arc_ball>();
    the_world.registerComponent<test_rotation_comp>();

    auto& cam = Camera2D::Main.get<Camera2D>();
    cam.clearColorEnabled = true;
    cam.clearColor = float4{0xFF666666_argb};

    SampleIntegrations::initializePlugins();
}

void DemoApp::preload() {
    basic_application::preload();
}

void DemoApp::update_frame(float dt) {
    basic_application::update_frame(dt);

    scene_pre_update(root, dt);

    if (currentSample) {
        currentSample->update(dt);
    }

    //update_game_title_start(w);
    scene_post_update(root);
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
    EK_DEBUG << "Start Demo: prepareInternalResources";
    SampleText::prepareInternalResources();

    SampleBase::samplesContainer = createNode2D("sample");
    append(game, SampleBase::samplesContainer);

    auto prev = createButton("<", [] { scrollSample(-1); });
    auto next = createButton(">", [] { scrollSample(+1); });
    prev.assign<LayoutRect>().enableAlignX(0, 30);
    next.assign<LayoutRect>().enableAlignX(1, -30);

    tfSampleTitle = createNode2D("title");
    addText(tfSampleTitle, "");
    tfSampleTitle.assign<LayoutRect>().enableAlignX(0.5);

    auto controls = createNode2D("controls");
    append(controls, tfSampleTitle);
    append(controls, prev);
    append(controls, next);
    controls.assign<LayoutRect>().enableAlignY(1, -30);

    append(game, controls);

    EK_DEBUG << "Start Demo: initSamples";
    initSamples();
}

DemoApp::~DemoApp() = default;
