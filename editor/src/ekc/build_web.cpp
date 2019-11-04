#include "project_config.h"
#include "template_vars.h"

#include <ek/fs/system.hpp>
#include <ek/fs/working_dir.hpp>

namespace ekc {

using namespace ek;

void build_web(const project_config_t& project) {
    using ek::path_join;

    execute("mkdir cmake-build-wasm-release");

    working_dir_t::with("cmake-build-wasm-release", [&]() {
        execute("cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=" +
                project.path_emsdk_toolchain.str());
        execute("make " + project.cmake_target + " -j9");

        auto tpl_dir = project.path_ekx / "editor/templates/web";
        auto output_dir = project.path / project.build_dir;
        template_vars_t tpl_context;
        fill_template_vars(project, tpl_context);
        copy_template(tpl_context, tpl_dir / "index.html.mustache", output_dir / "index.html");
        copy_template(tpl_context, tpl_dir / "manifest.json.mustache",
                      output_dir / "manifest.webmanifest");
        copy_template(tpl_context, tpl_dir / "sw.js.mustache", output_dir / "sw.js");
        copy_file(tpl_dir / "pwacompat.min.js", output_dir / "pwacompat.min.js");
        copy_file(tpl_dir / "howler.core.min.js", output_dir / "howler.core.min.js");
        copy_file(tpl_dir / "audio_mini.js", output_dir / "audio_mini.js");

        if (!project.html.deploy_dir.empty()) {
            copy_file(output_dir / "pwacompat.min.js",
                      project.html.deploy_dir / "pwacompat.min.js");

            copy_file(output_dir / "howler.core.min.js",
                      project.html.deploy_dir / "howler.core.min.js");

            copy_file(output_dir / "manifest.webmanifest",
                      project.html.deploy_dir / "manifest.webmanifest");
            copy_file(output_dir / "sw.js", project.html.deploy_dir / "sw.js");
            copy_file(output_dir / "index.html", project.html.deploy_dir / "index.html");
            copy_file(output_dir / (project.binary_name + ".wasm"),
                      project.html.deploy_dir / (project.binary_name + ".wasm"));
            copy_file(output_dir / (project.binary_name + ".js"),
                      project.html.deploy_dir / (project.binary_name + ".js"));
            copy_file(output_dir / (project.binary_name + ".data"),
                      project.html.deploy_dir / (project.binary_name + ".data"));

            // TODO: icons / assets
        }
    });
}
}