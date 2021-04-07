#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/util/logger.hpp>
#include <ek/timers.hpp>
#include <ek/imaging/ImageSubSystem.hpp>
#include <ek/Allocator.hpp>
#include <ek/ds/Array.hpp>

#include <ek/editor/marketing/export_marketing.hpp>
#include <ek/editor/assets/editor_project.hpp>

using namespace ek;

// Marketing Resources generation:
// 1) Export command
// - what to export
// - platform
// - file/dir
// - type
// - output
// `ekc export market assets/res web generated/web`
int main(int argc, char** argv) {
    using namespace std;

    memory::initialize();
    clock::initialize();
    imaging::initialize();

    EK_INFO << "== EKC util ==";
    EK_INFO << "Executable path: " << get_executable_path();
    EK_INFO << "Working dir: " << current_working_directory();
    EK_INFO << "Arguments: ";

    Array<std::string> args{};
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
        EK_INFO << "    " << argv[i];
    }

    if (argc <= 1) {
        EK_INFO << "version: 0.0.1";
        return 0;
    }

    if (argc > 1) {
        if (args[1] == "export") {
            if (argc > 2) {
                const std::string& what = args[2];
                if (what == "market") {
                    marketing_asset_t marketing_data{};
                    marketing_data.input = path_t{args[3]};
                    marketing_data.commands.push_back({
                                                              args[4],
                                                              path_t{args[5]}
                                                      });
                    process_market_asset(marketing_data);
                } else if (what == "assets") {
                    editor_project_t project{};
                    project.devMode = false;
                    if (getenv("DEV_ASSETS") != nullptr) {
                        EK_WARN << "- DEV_ASSETS detected: export includes development resources";
                        project.devMode = true;
                    }
                    project.base_path = path_t{args[3]};
                    project.populate();
                    project.build(path_t{args[4]});
                }
            }
        } else if (args[1] == "prerender_flash") {
            runFlashFilePrerender(args);
        }
    }

    return 0;
}