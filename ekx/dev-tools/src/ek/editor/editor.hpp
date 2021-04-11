#pragma once

#include <ek/scenex/asset2/asset_manager.hpp>
#include <ek/editor/imgui/imgui_module.hpp>

namespace ek {

struct EditorSettings {
    bool notifyAssetsOnScaleFactorChanged = true;
    bool showEditor = true;
    bool showImGuiDemoWindow = false;
    bool showHierarchyWindow = true;
    bool showInspectorWindow = true;
    bool showStatsWindow = false;
    bool showResourcesView = false;
    float2 windowSize = float2::zero;
    bool dirty = false;

    void save() const;

    void load();
};

class basic_application;

class Editor {
public:
    explicit Editor(basic_application& app);

    ~Editor();

    void onEvent(const app::event_t& event);
    void onFrameCompleted();

public:
    static EditorSettings settings;
    inline static bool inspectorEnabled = false;

    static void invalidateSettings();

    imgui_module_t gui_;

private:

    signal_t<>::Listener t1;
    signal_t<float>::Listener t2;
    signal_t<>::Listener t3;

    basic_application* app_;

public:

    static void initialize();
};

}