#include "editor.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <ek/editor/gui/gui.hpp>
#include <pugixml.hpp>
#include <ek/util/logger.hpp>

namespace ek {

using app::g_app;

void editor_onFrameCompleted() {
    resolve<Editor>().onFrameCompleted();
}

void editor_onEvent(const app::event_t& e) {
    resolve<Editor>().onEvent(e);
}

Editor::Editor(basic_application& app) :
        app_{&app} {

    g_app.on_frame_completed += editor_onFrameCompleted;
    g_app.on_event += editor_onEvent;

    app.hook_on_preload.add([this]() {
        project.update_scale_factor(app_->scale_factor, false);
        project.populate();
    });
    t2 = app.hook_on_update.add([this](float dt) {
        project.update_scale_factor(app_->scale_factor, settings.notifyAssetsOnScaleFactorChanged);
        gui_.begin_frame(dt);
        if (settings.showEditor) {
            guiEditor(*this);
        }
    });
    t1 = app.hook_on_draw_frame.add([this]() {
        gui_.end_frame();
    });
    t3 = app.hook_on_render_frame.add([this]() {

    });

    app.preloadOnStart = false;
}

Editor::~Editor() {
    app_->hook_on_update.remove(t2);
    app_->hook_on_draw_frame.remove(t1);
    app_->hook_on_render_frame.remove(t3);

    g_app.on_frame_completed -= editor_onFrameCompleted;
    g_app.on_event -= editor_onEvent;
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
    service_locator_instance<Editor>::init(resolve<basic_application>());
}

const char* editorSettingsPath = "editor_settings.xml";

EditorSettings Editor::settings{};

void EditorSettings::save() const {
    pugi::xml_document xml;

    auto node = xml.append_child("editor");
    node.append_attribute("notifyAssetsOnScaleFactorChanged").set_value(notifyAssetsOnScaleFactorChanged);
    node.append_attribute("showEditor").set_value(showEditor);
    node.append_attribute("showHierarchyWindow").set_value(showHierarchyWindow);
    node.append_attribute("showInspectorWindow").set_value(showInspectorWindow);
    node.append_attribute("showStatsWindow").set_value(showStatsWindow);
    node.append_attribute("showResourcesView").set_value(showResourcesView);
    node.append_attribute("showAssetsView").set_value(showAssetsView);
    node.append_attribute("showBuildWindow").set_value(showBuildWindow);
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
    pugi::xml_document doc{};
    if (!doc.load_file(editorSettingsPath)) {
        EK_ERROR << "Can't load editor settings";
        return;
    }
    auto node = doc.first_child();
    notifyAssetsOnScaleFactorChanged = node.attribute("notifyAssetsOnScaleFactorChanged").as_bool(
            notifyAssetsOnScaleFactorChanged);
    showEditor = node.attribute("showEditor").as_bool(showEditor);
    showHierarchyWindow = node.attribute("showHierarchyWindow").as_bool(showHierarchyWindow);
    showInspectorWindow = node.attribute("showInspectorWindow").as_bool(showInspectorWindow);
    showStatsWindow = node.attribute("showStatsWindow").as_bool(showStatsWindow);
    showResourcesView = node.attribute("showResourcesView").as_bool(showResourcesView);
    showAssetsView = node.attribute("showAssetsView").as_bool(showAssetsView);
    showBuildWindow = node.attribute("showBuildWindow").as_bool(showBuildWindow);
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

}