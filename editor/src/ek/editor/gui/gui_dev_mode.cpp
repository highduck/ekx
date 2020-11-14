#include <imgui.h>
#include <ek/editor/editor.hpp>
#include "gui_dev_mode.hpp"
#include "gui.hpp"

namespace ek {

void gui_dev_mode(Editor& editor) {

    auto& settings = editor.settings;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("Hierarchy", nullptr, &settings.showHierarchyWindow);
            ImGui::MenuItem("Inspector", nullptr, &settings.showInspectorWindow);
            ImGui::MenuItem("Stats", nullptr, &settings.showStatsWindow);
            ImGui::MenuItem("Reload Assets on Scale", nullptr, &settings.notifyAssetsOnScaleFactorChanged);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("ImGui")) {
            ImGui::MenuItem("Demo", nullptr, &settings.showImGuiDemoWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    if(settings.showImGuiDemoWindow) ImGui::ShowDemoWindow(&settings.showImGuiDemoWindow);
    if(settings.showHierarchyWindow) guiHierarchyWindow(&settings.showHierarchyWindow);
    if(settings.showInspectorWindow) guiInspectorWindow(&settings.showInspectorWindow);
    if(settings.showStatsWindow) guiStatsWindow(&settings.showStatsWindow);
}

}