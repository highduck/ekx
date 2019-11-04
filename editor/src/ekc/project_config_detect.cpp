#include "project_config.h"

#include <ek/fs/working_dir.hpp>

#include <cstdlib>
#include <ek/logger.hpp>

namespace ekc {

void init_project_config_defaults(project_config_t& dest) {
    const auto* ekc_root = std::getenv("EKC_ROOT");

    if (!ekc_root) {
        EK_ERROR << "Please define EKC_ROOT environment variable. Abort.";
        abort();
    }

    dest.path_ekc = ek::path_t{ekc_root};
    dest.path_emsdk_toolchain = ek::path_t{
            "/Users/ilyak/dev/emsdk/emscripten/1.38.29/cmake/Modules/Platform/Emscripten.cmake"};
    dest.path = ek::path_t{ek::current_working_directory()};
}

}