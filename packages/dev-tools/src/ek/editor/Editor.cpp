#include "Editor.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <pugixml.hpp>
#include <ek/log.h>
#include <ek/assert.h>

namespace ek {

void Editor::onRenderOverlay() {
    gui_.end_frame();

    bool dirty = false;
    for (auto* wnd: windows) {
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
    gui_.begin_frame((float)g_game_app->frameTimer.deltaTime);
    if (g_editor_config->showEditor) {
        drawGUI();
    }
}

void Editor::onBeforeFrameBegin() {
    auto& display = g_game_app->display;
    if (g_editor_config->showEditor && !display.simulated) {
        display.simulated = true;
    } else if (!g_editor_config->showEditor && display.simulated) {
        display.simulated = false;
    }
}

Editor::Editor() {
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
    g_game_app->dispatcher.listeners.push_back(this);
}

Editor::~Editor() {
    g_game_app->dispatcher.listeners.remove(this);
}

void Editor::load() {
    EK_DEBUG("load editor layout state");
    pugi::xml_document doc{};
    if (!doc.load_file("EditorLayoutState.xml")) {
        return;
    }
    auto node = doc.first_child();
    for (auto* wnd: windows) {
        wnd->load(node);
    }
}

void Editor::save() {
    pugi::xml_document xml;
    auto node = xml.append_child("EditorLayoutState");
    for (auto* wnd: windows) {
        wnd->save(node);
    }
    xml.save_file("EditorLayoutState.xml");
}

void Editor::onPostFrame() {
    gui_.on_frame_completed();
    invalidateSettings();
}

void Editor::onEvent(const ek_app_event& event) {
    EK_ASSERT(g_editor_config);
    auto& settings = *g_editor_config;
    switch (event.type) {
        case EK_APP_EVENT_KEY_DOWN:
            if (event.key.code == EK_KEYCODE_A &&
                (event.key.modifiers & EK_KEY_MOD_CONTROL) &&
                (event.key.modifiers & EK_KEY_MOD_SHIFT)) {
                settings.showEditor = !settings.showEditor;
                settings.dirty = true;
            }
            break;
        case EK_APP_EVENT_RESIZE: {
            const auto width = (int) (ek_app.viewport.width / ek_app.viewport.scale);
            const auto height = (int) (ek_app.viewport.height / ek_app.viewport.scale);
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

const char* editorSettingsPath = "editor_settings.xml";

void EditorSettings::save() const {
    pugi::xml_document xml;

    auto node = xml.append_child("editor");
    node.append_attribute("notifyAssetsOnScaleFactorChanged").set_value(notifyAssetsOnScaleFactorChanged);
    node.append_attribute("showEditor").set_value(showEditor);
    auto wnd = node.append_child("window");
    wnd.append_attribute("width").set_value(width);
    wnd.append_attribute("height").set_value(height);
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
    width = wnd.attribute("width").as_int((int) ek_app.config.width);
    height = wnd.attribute("height").as_int((int) ek_app.config.height);
}

void Editor::invalidateSettings() {
    auto& settings = *g_editor_config;
    if (settings.dirty) {
        ++settings.auto_save_counter;
        if (settings.auto_save_counter > 20) {
            settings.auto_save_counter = 0;
            settings.save();
            settings.dirty = false;
        }
    }
}

void Editor::onPreRender() {
    scene.onPreRender();
}

}

ek::Editor* g_editor = nullptr;
void init_editor(void) {
    EK_ASSERT(!g_editor);
    g_editor = new ek::Editor();
}

ek::EditorSettings* g_editor_config = nullptr;
void init_editor_config(void) {
    g_editor_config = new ek::EditorSettings();
    g_editor_config->load();
}
