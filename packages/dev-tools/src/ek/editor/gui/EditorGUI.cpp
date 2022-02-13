#include <ek/editor/Editor.hpp>

#include <ek/scenex/app/basic_application.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ekx/app/input_state.h>
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
#include "test_window.hpp"

namespace ek {

void Editor::drawGUI() {
    auto* baseApp = g_game_app;
    if (baseApp) {
        hierarchy.root = baseApp->root;
    }

    static bool fontIconsWindow = false;
    static bool imGuiDemoWindow = false;
    static bool testWindow = false;
    auto& settings = *g_editor_config;
    bool resetLayout = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            settings.dirty |= ImGui::MenuItem("Reload Assets on Scale", nullptr,
                                              &settings.notifyAssetsOnScaleFactorChanged);
            ///
            ImGui::Separator();
            ImGui::MenuItem("Emulate Touch Input", nullptr, &g_input_state.emulateTouch);
            ///
            ImGui::Separator();
            if (g_game_app) {
                ImGui::Text("User Insets Absolute");
                ImGui::DragFloat4("##userInsetsAbsolute",
                                  g_game_app->display.info.userInsetsAbsolute.data);
                ImGui::Text("User Insets Relative");
                ImGui::SliderFloat4("##userInsetsRelative",
                                    g_game_app->display.info.userInsetsRelative.data, 0.0f,1.0f);
            }

            ImGui::Separator();

            if (ImGui::BeginMenu(ICON_FA_FEATHER_ALT " Sokol")) {
                auto& sokol_gfx_gui_state = gui_.sokol_gfx_gui_state;
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
            ImGui::MenuItem("Functions", nullptr, &testWindow);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            for (auto* wnd : windows) {
                wnd->mainMenu();
            }
            if (ImGui::MenuItem("Reset Layout")) {
                resetLayout = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {

            ImGui::EndMenu();
        }
        ImGui::SameLine((ImGui::GetWindowContentRegionMax().x / 2.0f) -
                        (1.5f * (ImGui::GetFontSize() + ImGui::GetStyle().ItemSpacing.x)));

        if (ImGui::Button(game.paused ? ICON_FA_PLAY : ICON_FA_PAUSE)) {
            game.paused = !game.paused;
            game.dirty = true;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        game.dirty |= ImGui::SliderFloat(ICON_FA_CLOCK, &game.timeScale, 0.0f, 3.0f, "%.3f", 1.0f);
        ImGui::SameLine();
        game.dirty |= ImGui::Checkbox(ICON_FA_STOPWATCH, &game.profiler);

        s_profile_metrics.enabled = game.profiler;
        g_time_layers[TIME_LAYER_ROOT].scale = game.paused ? 0.0f : game.timeScale;

        ImGui::EndMainMenuBar();
    }
    auto dockSpaceId = ImGui::DockSpaceOverViewport();
    if (resetLayout) {
        ImGui::DockBuilderRemoveNode(dockSpaceId);
        ImGui::DockBuilderAddNode(dockSpaceId);
        ImVec2 size;
        size.x = ImGui::GetIO().DisplaySize.x * ImGui::GetIO().DisplayFramebufferScale.x;
        size.y = ImGui::GetIO().DisplaySize.y * ImGui::GetIO().DisplayFramebufferScale.y;
        ImGui::DockBuilderSetNodeSize(dockSpaceId, size);
        ImGuiID DockBottom = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Down, 0.3f, nullptr, &dockSpaceId);
        ImGuiID DockLeft = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.2f, nullptr, &dockSpaceId);
        ImGuiID DockRight = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Right, 0.20f, nullptr, &dockSpaceId);

        ImGuiID DockLeftChild = ImGui::DockBuilderSplitNode(DockLeft, ImGuiDir_Down, 0.875f, nullptr, &DockLeft);
        ImGuiID DockRightChild = ImGui::DockBuilderSplitNode(DockRight, ImGuiDir_Down, 0.875f, nullptr, &DockRight);
        ImGuiID DockingLeftDownChild = ImGui::DockBuilderSplitNode(DockLeftChild, ImGuiDir_Down, 0.06f, nullptr,
                                                                   &DockLeftChild);
        ImGuiID DockingRightDownChild = ImGui::DockBuilderSplitNode(DockRightChild, ImGuiDir_Down, 0.06f, nullptr,
                                                                    &DockRightChild);

        ImGuiID DockBottomChild = ImGui::DockBuilderSplitNode(DockBottom, ImGuiDir_Down, 0.2f, nullptr, &DockBottom);
        ImGuiID DockingBottomLeftChild = ImGui::DockBuilderSplitNode(DockBottomChild, ImGuiDir_Left, 0.5f, nullptr,
                                                                     &DockBottomChild);
        ImGuiID DockingBottomRightChild = ImGui::DockBuilderSplitNode(DockBottomChild, ImGuiDir_Right, 0.5f, nullptr,
                                                                      &DockBottomChild);

        ImGuiID DockMiddle = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 1.0f, nullptr, &dockSpaceId);

        ImGui::DockBuilderDockWindow("###SceneWindow", DockMiddle);
        ImGui::DockBuilderDockWindow("###GameWindow", DockMiddle);

        ImGui::DockBuilderDockWindow("###HierarchyWindow", DockLeft);
        ImGui::DockBuilderDockWindow("###InspectorWindow", DockRight);
        ImGui::DockBuilderDockWindow("###ConsoleWindow", DockingBottomLeftChild);
    }

    for (auto* wnd : windows) {
        wnd->show();
    }

    hierarchy.validateSelection();
    inspector.list = hierarchy.selection;

    if (imGuiDemoWindow) ImGui::ShowDemoWindow(&imGuiDemoWindow);
    if (fontIconsWindow) showFontIconsPreview(&fontIconsWindow);
    if (testWindow) showTestWindow(&testWindow);
}

}