#include <ekc/project_config.hpp>
#include <ekc/ekc.hpp>

#include <ek/editor/imgui/imgui.hpp>
#include <ek/system/system.hpp>
#include <ek/system/working_dir.hpp>

#include <ek/logger.hpp>

using namespace ekc;

namespace ek {

project_config_t* project_config_ = nullptr;

void gui_ekc() {
    ImGui::Begin("EKC");

    if (!project_config_ || ImGui::Button("Reload Config")) {
        delete project_config_;
        project_config_ = new project_config_t;

        working_dir_t::with("..", []() {
            create_project_config(*project_config_);
        });

        EK_INFO << "exe: " << get_executable_path();
        EK_INFO << "cwd: " << current_working_directory();
    }

    if (project_config_) {
        if (ImGui::Button("Export Web")) {
            working_dir_t::with("..", []() {
                build_web(*project_config_);
            });
        }
        if (ImGui::Button("Export Android")) {
            working_dir_t::with("..", []() {
                create_android_project(*project_config_);
            });
        }
        if (ImGui::Button("Export iOS")) {
            working_dir_t::with("..", []() {
                create_xcode_ios(*project_config_);
            });
        }
        if (ImGui::Button("Export Marketing")) {
            working_dir_t::with("..", []() {
                process_assets(*project_config_);
            });
        }
    }

    ImGui::End();
}

}