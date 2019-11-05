#include <ekc/project_config.h>

#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/logger.hpp>
#include <platform/boot.h>
#include <ekc/ekc.hpp>

using namespace ekc;

namespace ek {

void main() {
    EK_INFO << "exe: " << get_executable_path();
    EK_INFO << "cwd: " << current_working_directory();
    
    project_config_t project;
    create_project_config(project);

    const auto& args = get_program_arguments();
    for (const auto& arg : args) {
        if (arg == "prepare") {
            process_assets(project);
        } else if (arg == "web") {
            build_web(project);
        } else if (arg == "android") {
            create_android_project(project);
        } else if (arg == "ios") {
            create_xcode_ios(project);
        }
    }
}

}

void ek_main() {

}