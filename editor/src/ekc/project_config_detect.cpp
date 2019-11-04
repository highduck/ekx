#include "project_config.h"

#include <ek/fs/working_dir.hpp>
#include <ek/logger.hpp>
#include <ek/fs/system.hpp>

#include <cstdlib>

namespace ekc {

void init_project_config_defaults(project_config_t& dest) {
    const auto* ekx_root = std::getenv("EKX_ROOT");

    if (!ekx_root) {
        EK_ERROR << "Please define EKX_ROOT environment variable. Abort.";
        abort();
    }
    dest.path_ekx = ek::path_t{ekx_root};

    if (!ek::is_dir(dest.path_ekx)) {
        EK_ERROR << "EKX_ROOT is not a directory: " << dest.path_ekx;
        abort();
    }


    dest.path_emsdk_toolchain = ek::path_t{
            "/Users/ilyak/dev/emsdk/emscripten/1.38.29/cmake/Modules/Platform/Emscripten.cmake"};
    dest.path = ek::path_t{ek::current_working_directory()};
}

}