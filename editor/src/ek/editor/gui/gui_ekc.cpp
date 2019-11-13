#include <ek/editor/imgui/imgui.hpp>
#include <ek/system/system.hpp>
#include <ek/system/working_dir.hpp>

#include <ek/logger.hpp>
#include <ek/editor/marketing/export_marketing.hpp>

namespace ek {

void gui_ekc() {
    ImGui::Begin("EKC");

    if (ImGui::Button("Reload Config")) {
        EK_INFO << "exe: " << get_executable_path();
        EK_INFO << "cwd: " << current_working_directory();
    }

    if (ImGui::Button("Export Web")) {
        EK_WARN << "Not supported yet. Use <ekx/cli> tools";
    }
    if (ImGui::Button("Export Android")) {
        EK_WARN << "Not supported yet. Use <ekx/cli> tools";
    }
    if (ImGui::Button("Export iOS")) {
        EK_WARN << "Not supported yet. Use <ekx/cli> tools";
    }
    if (ImGui::Button("Export Marketing")) {
        working_dir_t::with("..", []() {
            process_market_asset(marketing_asset_t{});
        });
    }

    ImGui::End();
}

}