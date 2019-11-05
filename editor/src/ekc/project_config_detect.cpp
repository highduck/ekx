#include "project_config.h"

#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/logger.hpp>

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
            "/Users/ilyak/dev/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"};

    if (!ek::is_file(dest.path_emsdk_toolchain)) {
        EK_WARN << "Emscripten SDK toolchain is not found: " << dest.path_ekx;
        EK_WARN << "Web Target is not available";
    }
    
    dest.path = ek::path_t{ek::current_working_directory()};
}

}