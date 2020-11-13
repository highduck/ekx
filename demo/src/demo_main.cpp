#include "demo_main.hpp"
#include "sample_text.hpp"
#include "sample_flash.hpp"
#include "3d/sample_3d.hpp"
#include "piko/sample_piko.hpp"

#include <ek/math/rand.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/scenex/scene_system.hpp>
#include <ek/scenex/asset2/builtin_assets.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/components/button.hpp>
#include <ek/scenex/components/interactive.hpp>
#include <ek/scenex/components/layout.hpp>

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
    get_drawable<drawable_text>(tfSampleTitle).text = currentSample->title;
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
        base_app_type() {
}

void DemoApp::initialize() {
    base_app_type::initialize();
}

void DemoApp::preload() {
    base_app_type::preload();

    clear_color_enabled = true;

    SampleText::prepareInternalResources();
}

void DemoApp::update_frame(float dt) {
    base_app_type::update_frame(dt);

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

    draw_node(root);
}

drawable_text& addText(ecs::entity e, const char* text) {
    auto* tf = new drawable_text();
    tf->format.font.setID("mini");
    tf->format.size = 16;
    tf->format.addShadow(0x0_rgb, 8);
    tf->text = text;
    tf->format.setAlignment(Alignment::Center);
    ecs::get_or_create<display_2d>(e).drawable.reset(tf);
    return *tf;
}

template<typename Func>
ecs::entity createButton(const char* label, Func fn) {
    auto e = create_node_2d(label);
    auto& tf = addText(e, label);
    tf.fillColor = 0x77000000_argb;
    tf.borderColor = 0x77FFFFFF_argb;
    tf.hitFullBounds = true;
    tf.rect.set(-20, -20, 40, 40);
    ecs::assign<interactive_t>(e).cursor = app::mouse_cursor::button;
    ecs::assign<button_t>(e).clicked += fn;
    return e;
}

void DemoApp::start_game() {
//    setup_game(w, game);

    SampleBase::samplesContainer = create_node_2d("sample");
    append(game, SampleBase::samplesContainer);

    auto prev = createButton("<", [] { scrollSample(-1); });
    auto next = createButton(">", [] { scrollSample(+1); });
    auto& l = ecs::assign<layout_t>(prev);
    l.align_x = true;
    l.x = {0, 30};
    auto& r = ecs::assign<layout_t>(next);
    r.align_x = true;
    r.x = {1, -30};

    tfSampleTitle = create_node_2d("title");
    addText(tfSampleTitle, "");
    auto& t = ecs::assign<layout_t>(tfSampleTitle);
    t.align_x = true;
    t.x = {0.5, 0};

    auto controls = create_node_2d("controls");
    append(controls, tfSampleTitle);
    append(controls, prev);
    append(controls, next);
    auto& c = ecs::assign<layout_t>(controls);
    c.align_y = true;
    c.y = {1, -30};

    append(game, controls);

    sampleFactory.emplace_back([] { return new SampleText(); });
    sampleFactory.emplace_back([] { return new Sample3D(); });
    sampleFactory.emplace_back([] { return new SamplePiko(); });
    sampleFactory.emplace_back([] { return new SampleFlash("test1"); });
    sampleFactory.emplace_back([] { return new SampleFlash("test2"); });
    setCurrentSample(0);
}

DemoApp::~DemoApp() = default;
