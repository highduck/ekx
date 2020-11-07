#include "picos_main.hpp"
#include "sample_base.hpp"
#include "sample_text.hpp"
#include "3d/sample_3d.hpp"

#include <ek/scenex/systems/main_flow.hpp>
#include <sim/driver_node_script.h>
#include <ek/scenex/scene_system.hpp>
#include <ek/math/rand.hpp>
#include <ek/scenex/data/sg_factory.hpp>
#include <ek/scenex/asset2/builtin_assets.hpp>
#include <ek/scenex/text/truetype_font.hpp>
#include <ek/scenex/app/input_controller.hpp>
#include <piko/sample_piko.hpp>

namespace ek {

std::vector<std::function<SampleBase*()>> sampleFactory;
int currentSampleIndex = 0;
std::unique_ptr<SampleBase> currentSample = nullptr;

void main() {
    app::g_app.window_cfg = {"Piko", {1024, 768}};
    app::g_app.on_device_ready << [] {
        run_app<PikoApp>();
    };
    start_application();
}
}

using namespace ek;

PikoApp::PikoApp()
        : base_app_type() {
}

void PikoApp::initialize() {
    base_app_type::initialize();
}

void PikoApp::preload() {
    base_app_type::preload();

    clear_color_enabled = true;

    SampleText::prepareInternalResources();

    sampleFactory.emplace_back([] { return new SampleText(); });
    sampleFactory.emplace_back([] { return new Sample3D(); });
    sampleFactory.emplace_back([] { return new SamplePiko(); });

    currentSample.reset(sampleFactory[currentSampleIndex]());
}

void PikoApp::update_frame(float dt) {
    base_app_type::update_frame(dt);

    auto& ic = resolve<input_controller>();
    int samplesCount = static_cast<int>(sampleFactory.size());
    if (ic.is_key_down(app::key_code::ArrowRight)) {
        ++currentSampleIndex;
        if (currentSampleIndex >= samplesCount) {
            currentSampleIndex = 0;
        }
        currentSample.reset(sampleFactory[currentSampleIndex]());
    } else if (ic.is_key_down(app::key_code::ArrowLeft)) {
        --currentSampleIndex;
        if (currentSampleIndex < 0) {
            currentSampleIndex = samplesCount - 1;
        }
        currentSample.reset(sampleFactory[currentSampleIndex]());
    }

    scene_pre_update(root, dt);

    if (currentSample) {
        currentSample->update(dt);
    }

    //update_game_title_start(w);
    scene_post_update(root, dt);
}

void PikoApp::render_frame() {
//    base_app_type::render_frame();
    if (!started_) return;

    if (currentSample) {
        currentSample->draw();
    }

    draw_node(root);
}

void PikoApp::start_game() {
//    setup_game(w, game);

//    append(game, sg_create("tests", "test"));
}

PikoApp::~PikoApp() = default;
