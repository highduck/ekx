#include <cstring>

#include <ek/system/working_dir.hpp>
#include <ek/debug.hpp>
#include <ek/timers.hpp>
#include <ek/ds/Array.hpp>
#include <ek/core.hpp>

#include "ek/bmfont.hpp"
#include "ek/flash.hpp"
#include "ek/obj.hpp"
#include "ek/atlas.hpp"

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

    ek::core::setup();

    EK_INFO << "== EKC: 0.1.8 ==";

    Array<std::string> args{};
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    if (argc > 1) {
        EK_INFO << "Command: " << args[1];
        if (args[1] == "prerender_flash") {
            runFlashFilePrerender(args);
        } else if (args[1] == "obj") {
            const std::string& input = args[2];
            const std::string& output = args[3];
            ek::convertObjModel(input.c_str(), output.c_str());
        } else if (args[1] == "bmfont") {
            ek::exportBitmapFont(args[2].c_str());
        } else if (args[1] == "flash") {
            ek::exportFlash(args[2].c_str());
        } else if (args[1] == "atlas") {
            ek::exportAtlas(args[2].c_str());
        }
    }

    return 0;
}