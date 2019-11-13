#include "project_config.hpp"
#include "template_vars.h"

#include <ek/system/system.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/logger.hpp>

namespace ekc {

using namespace ek;

void build_web(const project_config_t& project) {
    using ek::path_join;

    //    execute("source /Users/ilyak/dev/emsdk/emsdk_env.sh");
//        execute("cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=" +
//                project.path_emsdk_toolchain.str());
//        execute("make " + project.cmake_target + " -j12");

    auto output_dir = project.path.project / project.build_dir;
    template_vars_t tpl_context;
    fill_template_vars(project, tpl_context);

    for (const auto& templ : project.templates) {
        EK_DEBUG << "copy template: " << templ.source << " to " << templ.dest;
        if (templ.type == "tpl") {
            copy_template(tpl_context, templ.source, templ.dest);
        } else if (templ.type == "file") {
            copy_file(templ.source, templ.dest);
        } else if (templ.type == "tree") {
            copy_tree(templ.source, templ.dest);
        }
    }

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
}
}