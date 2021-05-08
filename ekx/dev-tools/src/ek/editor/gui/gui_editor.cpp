#include "gui.hpp"
#include <ek/editor/editor.hpp>
#include <ek/util/ServiceLocator.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include <imgui.h>
#include <ek/scenex/app/input_controller.hpp>
#include "FontIconsPreview.hpp"

// private impls:
#include "StatsWindow_impl.hpp"
#include "GameWindow_impl.hpp"
#include "HierarchyWindow_impl.hpp"
#include "InspectorWindow_impl.hpp"
#include "MemoryProfiler_impl.hpp"
#include "SceneWindow_impl.hpp"
#include "ConsoleWindow_impl.hpp"
#include "ResourcesWindow_impl.hpp"
#include "Widgets_impl.hpp"

namespace ek {

void guiEditor(Editor& editor) {
    auto* baseApp = Locator::get<basic_application>();
    if (baseApp) {
        editor.hierarchy.root = ecs::EntityRef{baseApp->root};
    }

    static bool fontIconsWindow = false;
    static bool imGuiDemoWindow = false;
    auto& settings = Editor::settings;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            settings.dirty |= ImGui::MenuItem("Reload Assets on Scale", nullptr,
                                              &settings.notifyAssetsOnScaleFactorChanged);
            ///
            ImGui::Separator();
            auto* ic = Locator::get<input_controller>();
            if (ic) {
                ImGui::MenuItem("Emulate Touch Input", nullptr, &ic->emulateTouch);
            }
            ///
            ImGui::Separator();
            auto* baseApp = Locator::get<basic_application>();
            if (baseApp) {
                ImGui::Text("User Insets Absolute");
                ImGui::DragFloat4("##userInsetsAbsolute", baseApp->display.info.userInsetsAbsolute.data());
                ImGui::Text("User Insets Relative");
                ImGui::SliderFloat4("##userInsetsRelative", baseApp->display.info.userInsetsRelative.data(), 0.0f,
                                    1.0f);
            }

            ImGui::Separator();

            if (ImGui::BeginMenu(ICON_FA_FEATHER_ALT " Sokol")) {
                auto& sokol_gfx_gui_state = editor.gui_.sokol_gfx_gui_state;
                ImGui::MenuItem("Capabilities", 0, &sokol_gfx_gui_state.caps.open);
                ImGui::MenuItem("Buffers", 0, &sokol_gfx_gui_state.buffers.open);
                ImGui::MenuItem("Images", 0, &sokol_gfx_gui_state.images.open);
                ImGui::MenuItem("Shaders", 0, &sokol_gfx_gui_state.shaders.open);
                ImGui::MenuItem("Pipelines", 0, &sokol_gfx_gui_state.pipelines.open);
                ImGui::MenuItem("Passes", 0, &sokol_gfx_gui_state.passes.open);
                ImGui::MenuItem("Calls", 0, &sokol_gfx_gui_state.capture.open);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("ImGui")) {
                ImGui::MenuItem("Demo", nullptr, &imGuiDemoWindow);
                ImGui::MenuItem("Editor Font Icons", nullptr, &fontIconsWindow);
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            for (auto* wnd : editor.windows) {
                wnd->mainMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {

            ImGui::EndMenu();
        }
        ImGui::SameLine((ImGui::GetWindowContentRegionMax().x / 2.0f) -
                        (1.5f * (ImGui::GetFontSize() + ImGui::GetStyle().ItemSpacing.x)));

        if (ImGui::Button(editor.game.paused ? ICON_FA_PLAY : ICON_FA_PAUSE)) {
            editor.game.paused = !editor.game.paused;
            editor.game.dirty = true;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        editor.game.dirty |= ImGui::SliderFloat(ICON_FA_CLOCK, &editor.game.timeScale, 0.0f, 3.0f, "%.3f", 1.0f);
        ImGui::SameLine();
        editor.game.dirty |= ImGui::Checkbox(ICON_FA_STOPWATCH, &editor.game.profiler);

        Locator::ref<basic_application>().profiler.enabled = editor.game.profiler;
        TimeLayer::Root->scale = editor.game.paused ? 0.0f : editor.game.timeScale;

        ImGui::EndMainMenuBar();
    }
    ImGui::DockSpaceOverViewport();

    for (auto* wnd : editor.windows) {
        wnd->show();
    }

    editor.hierarchy.validateSelection();
    editor.inspector.list = editor.hierarchy.selection;

    if (imGuiDemoWindow) ImGui::ShowDemoWindow(&imGuiDemoWindow);
    if (fontIconsWindow) showFontIconsPreview(&fontIconsWindow);
}

}