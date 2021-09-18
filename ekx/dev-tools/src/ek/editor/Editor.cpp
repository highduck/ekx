#include "Editor.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <pugixml.hpp>
#include <ek/debug.hpp>

namespace ek {

using app::g_app;

void Editor::onRenderOverlay() {
    gui_.end_frame();

    bool dirty = false;
    for (auto* wnd : windows) {
        dirty |= wnd->dirty;
    }
    if (dirty) {
        save();
    }
}

void Editor::onRenderFrame() {

}

void Editor::onUpdate() {
    //project.update_scale_factor(app_->scale_factor, settings.notifyAssetsOnScaleFactorChanged);
    gui_.begin_frame(app.frameTimer.deltaTime);
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

Editor::Editor(basic_application& app_) :
        app{app_} {

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
    app.dispatcher.listeners.push_back(this);
}

Editor::~Editor() {
    app.dispatcher.listeners.remove(this);
}

void Editor::load() {
    EK_DEBUG("load editor layout state");
    pugi::xml_document doc{};
    if (!doc.load_file("EditorLayoutState.xml")) {
        return;
    }
    auto node = doc.first_child();
    for (auto* wnd : windows) {
        wnd->load(node);
    }
}

void Editor::save() {
    pugi::xml_document xml;
    auto node = xml.append_child("EditorLayoutState");
    for (auto* wnd : windows) {
        wnd->save(node);
    }
    xml.save_file("EditorLayoutState.xml");
}

void Editor::onPostFrame() {
    gui_.on_frame_completed();
    invalidateSettings();
}

void Editor::onEvent(const app::Event& event) {
    using ek::app::EventType;

    switch (event.type) {
        case EventType::KeyDown:
            if (event.key.code == app::KeyCode::A &&
                event.key.isControl() &&
                event.key.isShift()) {
                settings.showEditor = !settings.showEditor;
                settings.dirty = true;
            }
            break;
        case EventType::Resize: {
            const float width = g_app.windowWidth;
            const float height = g_app.windowHeight;
            if (width != settings.width || height != settings.height) {
                settings.width = width;
                settings.height = height;
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
    if (width != g_app.config.width || height != g_app.config.height) {
        wnd.append_attribute("width").set_value(width);
        wnd.append_attribute("height").set_value(height);
    }
    if (!xml.save_file(editorSettingsPath)) {
        EK_ERROR("Can't save editor settings");
    }
}

void EditorSettings::load() {
    EK_DEBUG("loading editor settings");
    pugi::xml_document doc{};
    if (!doc.load_file(editorSettingsPath)) {
        EK_ERROR("Can't load editor settings");
        return;
    }
    auto node = doc.first_child();
    notifyAssetsOnScaleFactorChanged = node.attribute("notifyAssetsOnScaleFactorChanged").as_bool(
            notifyAssetsOnScaleFactorChanged);
    showEditor = node.attribute("showEditor").as_bool(showEditor);
    auto wnd = node.child("window");
    width = wnd.attribute("width").as_float(g_app.config.width);
    height = wnd.attribute("height").as_float(g_app.config.height);
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