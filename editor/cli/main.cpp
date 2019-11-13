#include <ekc/project_config.hpp>

#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/logger.hpp>
#include <ekc/process_market_assets.hpp>

using namespace ekc;

namespace ek {

void main() {
    EK_INFO << "exe: " << get_executable_path();
    EK_INFO << "cwd: " << current_working_directory();

    project_config_t project;
    create_project_config(project);
    process_market_asset(project);
}

}

void ek_main() {

}