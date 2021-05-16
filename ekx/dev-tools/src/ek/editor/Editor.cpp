#include "Editor.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <pugixml.hpp>
#include <ek/debug.hpp>

namespace ek {

using app::g_app;

void editor_onFrameCompleted() {
    Locator::ref<Editor>().onFrameCompleted();
}

void editor_onEvent(const app::event_t& e) {
    Locator::ref<Editor>().onEvent(e);
}

void Editor::onRenderOverlay() {
    gui_.end_frame();

    bool dirty = false;
    for(auto* wnd : windows) {
        dirty |= wnd->dirty;
    }
    if(dirty) {
        save();
    }
}

void Editor::onRenderFrame() {

}

void Editor::onUpdate() {
    //project.update_scale_factor(app_->scale_factor, settings.notifyAssetsOnScaleFactorChanged);
    gui_.begin_frame(app_->frameTimer.deltaTime);
    if (settings.showEditor) {
        drawGUI();
    }
}

void Editor::onBeforeFrameBegin() {
    auto& display = Locator::ref<basic_application>().display;
    if (settings.showEditor && !display.simulated) {
        display.simulated = true;
    } else if (!settings.showEditor && display.simulated) {
        display.simulated = false;
    }
}

Editor::Editor(basic_application& app) :
        app_{&app} {

    windows.push_back(&scene);
    windows.push_back(&game);
    windows.push_back(&inspector);
    windows.push_back(&hierarchy);
    // project
    windows.push_back(&console);
    windows.push_back(&stats);
    windows.push_back(&resources);
    windows.push_back(&memory);

    load();
    g_app.on_frame_completed += editor_onFrameCompleted;
    g_app.on_event += editor_onEvent;

    app.dispatcher.listeners.push_back(this);
}

Editor::~Editor() {
    app_->dispatcher.listeners.remove(this);
    g_app.on_frame_completed -= editor_onFrameCompleted;
    g_app.on_event -= editor_onEvent;
}

void Editor::load() {
    EK_TRACE("load editor layout state");
    pugi::xml_document doc{};
    if (!doc.load_file("EditorLayoutState.xml")) {
        return;
    }
    auto node = doc.first_child();
    for(auto* wnd : windows) {
        wnd->load(node);
    }
}

void Editor::save() {
    pugi::xml_document xml;
    auto node = xml.append_child("EditorLayoutState");
    for(auto* wnd : windows) {
        wnd->save(node);
    }
    xml.save_file("EditorLayoutState.xml");
}

void Editor::onFrameCompleted() {
    gui_.on_frame_completed();
    invalidateSettings();
}

void Editor::onEvent(const app::event_t& event) {
    using app::event_type;
    using app::key_code;
    switch (event.type) {
        case event_type::key_down:
            if (event.code == key_code::A && event.ctrl && event.shift) {
                settings.showEditor = !settings.showEditor;
                settings.dirty = true;
            }
            break;
        case event_type::app_resize: {
            const float2 windowSize{g_app.window_size};
            if (windowSize != settings.windowSize) {
                settings.windowSize = windowSize;
                settings.dirty = true;
            }
        }
            break;
        default:
            break;
    }
    gui_.on_event(event);
}

void Editor::initialize() {
    Locator::create<Editor>(Locator::ref<basic_application>());
}

const char* editorSettingsPath = "editor_settings.xml";

EditorSettings Editor::settings{};

void EditorSettings::save() const {
    pugi::xml_document xml;

    auto node = xml.append_child("editor");
    node.append_attribute("notifyAssetsOnScaleFactorChanged").set_value(notifyAssetsOnScaleFactorChanged);
    node.append_attribute("showEditor").set_value(showEditor);
    auto wnd = node.append_child("window");
    if (windowSize != float2{g_app.window_cfg.size}) {
        wnd.append_attribute("width").set_value(windowSize.x);
        wnd.append_attribute("height").set_value(windowSize.y);
    }
    if (!xml.save_file(editorSettingsPath)) {
        EK_ERROR << "Can't save editor settings";
    }
}

void EditorSettings::load() {
    EK_TRACE << "loading editor settings";
    pugi::xml_document doc{};
    if (!doc.load_file(editorSettingsPath)) {
        EK_ERROR << "Can't load editor settings";
        return;
    }
    auto node = doc.first_child();
    notifyAssetsOnScaleFactorChanged = node.attribute("notifyAssetsOnScaleFactorChanged").as_bool(
            notifyAssetsOnScaleFactorChanged);
    showEditor = node.attribute("showEditor").as_bool(showEditor);
    auto wnd = node.child("window");
    windowSize.x = wnd.attribute("width").as_float(g_app.window_cfg.size.x);
    windowSize.y = wnd.attribute("height").as_float(g_app.window_cfg.size.y);
}

void Editor::invalidateSettings() {
    static int autoSaveCounter = 0;

    if (settings.dirty) {
        ++autoSaveCounter;
        if (autoSaveCounter > 20) {
            autoSaveCounter = 0;
            settings.save();
            settings.dirty = false;
        }
    }
}

void Editor::onPreRender() {
    scene.onPreRender();
}

}