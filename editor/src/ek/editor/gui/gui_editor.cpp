#include <imgui.h>
#include <ek/editor/editor.hpp>
#include "gui.hpp"

namespace ek {

void guiEditor(Editor& editor) {

    auto& settings = editor.settings;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("Hierarchy", nullptr, &settings.showHierarchyWindow);
            ImGui::MenuItem("Inspector", nullptr, &settings.showInspectorWindow);
            ImGui::MenuItem("Stats", nullptr, &settings.showStatsWindow);
            ImGui::MenuItem("Reload Assets on Scale", nullptr, &settings.notifyAssetsOnScaleFactorChanged);
            ImGui::MenuItem("Resources View", nullptr, &settings.showResourcesView);
            ImGui::MenuItem("Project Assets", nullptr, &settings.showAssetsView);
            ImGui::MenuItem("Build & Export", nullptr, &settings.showBuildWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("ImGui")) {
            ImGui::MenuItem("Demo", nullptr, &settings.showImGuiDemoWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    if (settings.showImGuiDemoWindow) ImGui::ShowDemoWindow(&settings.showImGuiDemoWindow);
    if (settings.showHierarchyWindow) guiHierarchyWindow(&settings.showHierarchyWindow);
    if (settings.showInspectorWindow) guiInspectorWindow(&settings.showInspectorWindow);
    if (settings.showStatsWindow) guiStatsWindow(&settings.showStatsWindow);
    if (settings.showResourcesView) guiResourcesViewWindow(&settings.showResourcesView);
    if (settings.showAssetsView) guiAssetsViewWindow(&settings.showAssetsView, editor.project);
    if (settings.showBuildWindow) guiBuildWindow(&settings.showBuildWindow);

}

}