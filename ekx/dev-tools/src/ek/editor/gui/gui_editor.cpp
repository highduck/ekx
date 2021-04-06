#include "gui.hpp"
#include <ek/editor/editor.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/util/locator.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include <imgui.h>
#include <ek/scenex/app/input_controller.hpp>

namespace ek {

void guiEditor(Editor& editor) {

    auto& settings = Editor::settings;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Tools")) {
            settings.dirty |= ImGui::MenuItem("Hierarchy", nullptr, &settings.showHierarchyWindow);
            settings.dirty |= ImGui::MenuItem("Inspector", nullptr, &settings.showInspectorWindow);
            settings.dirty |= ImGui::MenuItem("Stats", nullptr, &settings.showStatsWindow);
            settings.dirty |= ImGui::MenuItem("Reload Assets on Scale", nullptr,
                                              &settings.notifyAssetsOnScaleFactorChanged);
            settings.dirty |= ImGui::MenuItem("Resources View", nullptr, &settings.showResourcesView);
            ImGui::Separator();
            if (ImGui::BeginMenu("Debug")) {
//                if (ImGui::DragFloat4("Add Insets", LayoutRect::AdditionalInsets.data())) {
//                    updateScreenRect(resolve<basic_application>().root);
//                }
                auto* ic = try_resolve<input_controller>();
                if (ic) {
                    ImGui::Checkbox("Touch Emulation", &ic->emulateTouch);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("ImGui")) {
            ImGui::MenuItem("Demo", nullptr, &settings.showImGuiDemoWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Sokol")) {
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
        ImGui::EndMainMenuBar();
    }
    if (settings.showImGuiDemoWindow) ImGui::ShowDemoWindow(&settings.showImGuiDemoWindow);
    if (settings.showHierarchyWindow) guiHierarchyWindow(&settings.showHierarchyWindow);
    if (settings.showInspectorWindow) guiInspectorWindow(&settings.showInspectorWindow);
    if (settings.showStatsWindow) guiStatsWindow(&settings.showStatsWindow);
    if (settings.showResourcesView) guiResourcesViewWindow(&settings.showResourcesView);
}

}