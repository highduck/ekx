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

    EK_INFO("== EKC: 0.1.8 ==");

    Array<std::string> args{};
    for (int i = 0; i < argc; ++i) {
        EK_INFO_F(" - %d : %s", i, argv[i]);
    }

    if (argc > 1) {
        std::string cmd = argv[1];
        EK_INFO_F("Command: %s", argv[1]);
        if (cmd == "prerender_flash") {
            runFlashFilePrerender(argc - 2, argv + 2);
        } else if (cmd == "obj") {
            const char* input = argv[2];
            const char* output = argv[3];
            ek::convertObjModel(input, output);
        } else if (cmd == "bmfont") {
            ek::exportBitmapFont(argv[2]);
        } else if (cmd == "flash") {
            ek::exportFlash(argv[2]);
        } else if (cmd == "atlas") {
            ek::exportAtlas(argv[2]);
        }
    }

    return 0;
}