#include "project_config.hpp"

#include <ek/fs/path.hpp>
#include <ek/system/system.hpp>
#include <ek/logger.hpp>
#include <ek/system/working_dir.hpp>

using ek::path_t;

namespace ekc {

void init_project_path(project_path_t& path) {
    const auto* ekx_root = std::getenv("EKX_ROOT");
    if (!ekx_root) {
        EK_ERROR << "Please define EKX_ROOT environment variable. Abort.";
        abort();
    }

    path.ekx = path_t{ekx_root};

    if (!ek::is_dir(path.ekx)) {
        EK_ERROR << "EKX_ROOT is not a directory: " << path.ekx;
        abort();
    }

    path.emsdk = path_t{"/Users/ilyak/dev/emsdk"};
    if (!ek::is_dir(path.emsdk)) {
        EK_WARN << "Emscripten SDK dir is not found: " << path.ekx;
        EK_WARN << "Web Target is not available";
        path.emsdk_toolchain = path.emsdk /
                               "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake";
        if (!ek::is_file(path.emsdk_toolchain)) {
            EK_WARN << "Emscripten SDK toolchain is not found: " << path.ekx;
            EK_WARN << "Web Target is not available";
        }
    }

    path.current_project = path.project = path_t{ek::current_working_directory()};
}

void create_project_config(project_config_t& out_config) {
    init_project_path(out_config.path);
}

}