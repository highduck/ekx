#include <imgui.h>
#include "gui_dev_mode.hpp"
#include "gui.hpp"
//#include "gui_window_stats.h"
//#include "editor_assets.hpp"

namespace ek {

static bool showImGuiDemoWindow = false;
static bool showHierarchyWindow = true;
static bool showInspectorWindow = true;
static bool showStatsWindow = false;

void gui_dev_mode() {

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("Hierarchy", nullptr, &showHierarchyWindow);
            ImGui::MenuItem("Inspector", nullptr, &showInspectorWindow);
            ImGui::MenuItem("Stats", nullptr, &showStatsWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("ImGui")) {
            ImGui::MenuItem("Demo", nullptr, &showImGuiDemoWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    if(showImGuiDemoWindow) ImGui::ShowDemoWindow(&showImGuiDemoWindow);
    if(showHierarchyWindow) guiHierarchyWindow(&showHierarchyWindow);
    if(showInspectorWindow) guiInspectorWindow(&showInspectorWindow);
    if(showStatsWindow) guiStatsWindow(&showStatsWindow);

    //    if (!hierarchySelectionList.empty()) {
//        if (ImGui::Button(hierarchySelectionList.size() > 1 ? "delete all" : "delete")) {
//            for (auto e : hierarchySelectionList) {
//                if (ecs::valid(e)) {
//                    if (ecs::has<node_t>(e)) {
//                        erase_node_component(e);
//                    }
//                    ecs::destroy(e);
//                }
//            }
//            hierarchySelectionList.clear();
//        }
//    } else {
//        ImGui::Button("dummy");
//    }

    //gui_draw_stats();
//    do_editor_debug_runtime_assets();
//    gui_ekc();
}

}