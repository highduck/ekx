import {path} from "ekx/deps.ts"
import {downloadFiles, getModuleDir, rm} from "ekx/utils/mod.ts"
import {buildMatrix} from "ekx/cmake/mod.ts"

const __dirname = getModuleDir(import.meta);

async function clean() {
    await rm(path.join(__dirname, "build"));
    await rm(path.join(__dirname, "src"));
}

async function fetch() {
    const imgui_branch = "docking";
    await downloadFiles({
        srcBaseUrl: `https://github.com/ocornut/imgui/raw/${imgui_branch}`,
        destPath: path.join(__dirname, "src"),
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

    const imgui_config = `
#define IMGUI_ENABLE_FREETYPE
#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define IMGUI_STB_TRUETYPE_FILENAME  <stb/stb_truetype.h>
#define IMGUI_STB_RECT_PACK_FILENAME  <stb/stb_rect_pack.h>

` + await Deno.readTextFile(path.join(__dirname, "src/imconfig.h"));
    await Deno.writeTextFile(path.join(__dirname, "src/imconfig.h"), imgui_config);

    const implot_branch = "master";
    await downloadFiles({
        srcBaseUrl: `https://github.com/epezent/implot/raw/${implot_branch}`,
        destPath: path.join(__dirname, "src"),
        fileList: [
            "implot.cpp",
            "implot.h",
            "implot_internal.h",
            "implot_items.cpp",
            "implot_demo.cpp"
        ]
    });

    const imguizmo_branch = "master";
    await downloadFiles({
        srcBaseUrl: `https://github.com/CedricGuillemet/ImGuizmo/raw/${imguizmo_branch}`,
        destPath: path.join(__dirname, "src/ImGuizmo"),
        fileList: [
            "GraphEditor.cpp",
            "GraphEditor.h",
            "ImCurveEdit.cpp",
            "ImCurveEdit.h",
            "ImGradient.cpp",
            "ImGradient.h",
            "ImGuizmo.cpp",
            "ImGuizmo.h",
            "ImSequencer.cpp",
            "ImSequencer.h",
            "ImZoomSlider.h",
        ]
    });
}

async function test() {
    await buildMatrix({
        cmakePath: "test",
        test: true,
        workingDir: __dirname
    });
    await rm(path.join(__dirname, "build"));
}

export default {clean, fetch, test};