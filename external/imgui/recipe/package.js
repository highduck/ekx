const {downloadFiles} = require('@ekx/cli-utils');
const fs = require("fs");

async function run() {
    console.info("imgui");
    const branch = "docking";
    await downloadFiles({
        srcBaseUrl: `https://github.com/ocornut/imgui/raw/${branch}`,
        destPath: "src",
        fileList: [
            "imconfig.h",
            "imgui_demo.cpp",
            "imgui.cpp",
            "imgui.h",
            "imgui_draw.cpp",
            "imgui_internal.h",
            "imgui_widgets.cpp",
            "imgui_tables.cpp",
            "imstb_textedit.h",
            "misc/freetype/imgui_freetype.cpp",
            "misc/freetype/imgui_freetype.h"
        ],
        fileMap: {
            "misc/cpp/imgui_stdlib.h": "imgui_stdlib.h",
            "misc/cpp/imgui_stdlib.cpp": "imgui_stdlib.cpp",
        }
    });

    let imguiconfig = fs.readFileSync("src/imconfig.h", "utf8");
    imguiconfig = `
#define IMGUI_ENABLE_FREETYPE
#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define IMGUI_STB_TRUETYPE_FILENAME  <stb/stb_truetype.h>
#define IMGUI_STB_RECT_PACK_FILENAME  <stb/stb_rect_pack.h>

` + imguiconfig;
    fs.writeFileSync("src/imconfig.h", imguiconfig);
}

run().catch(() => process.exit(1));