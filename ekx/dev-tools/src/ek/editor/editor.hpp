#pragma once

#include <ek/scenex/asset2/asset_manager.hpp>
#include "imgui/imgui_module.hpp"
#include "gui/HierarchyWindow.hpp"
#include "gui/InspectorWindow.hpp"
#include "gui/StatsWindow.hpp"
#include "gui/GameWindow.hpp"
#include "gui/MemoryProfiler.hpp"
#include "gui/SceneWindow.hpp"
#include "gui/ConsoleWindow.hpp"
#include "gui/ResourcesWindow.hpp"

namespace ek {

struct EditorSettings {
    bool notifyAssetsOnScaleFactorChanged = true;
    bool showEditor = true;
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

    HierarchyWindow hierarchy{};
    InspectorWindow inspector{};
    GameWindow game{};
    StatsWindow stats{};
    MemoryProfiler memory{};
    SceneWindow scene{};
    ConsoleWindow console{};
    ResourcesWindow resources{};
    Array<EditorWindow*> windows{};

    imgui_module_t gui_;

    void load();
    void save();

private:

    signal_t<>::Listener t1;
    signal_t<float>::Listener t2;
    signal_t<>::Listener t3;

    basic_application* app_;

public:

    static void initialize();
};

}