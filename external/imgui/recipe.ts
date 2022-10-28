import * as path from "path"
import {downloadFiles, getModuleDir, readTextFileSync, rm, writeTextFileSync} from "../../modules/utils/mod.js"

const __dirname = getModuleDir(import.meta);

async function clean() {
    await rm(path.join(__dirname, "src"));
    await rm(path.join(__dirname, "ttf"));
    await rm(path.join(__dirname, "include"));
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

` + readTextFileSync(path.join(__dirname, "src/imconfig.h"));
    writeTextFileSync(path.join(__dirname, "src/imconfig.h"), imgui_config);

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

    await downloadFiles({
        srcBaseUrl: `https://github.com/ocornut/imgui_club/raw/master`,
        destPath: path.join(__dirname, "src"),
        fileList: [
            "imgui_memory_editor/imgui_memory_editor.h",
        ]
    });

    await download_fonts();
    await download_font_headers();
}

function download_fonts(): Promise<void> {
    const destPath = path.join(__dirname, "ttf");
    return Promise.all([
        downloadFiles({
            srcBaseUrl: `https://github.com/ocornut/imgui/raw/docking`,
            destPath,
            fileMap: {
                "misc/fonts/Cousine-Regular.ttf": "Cousine-Regular.ttf"
            }
        }),
        downloadFiles({
            srcBaseUrl: `https://github.com/FortAwesome/Font-Awesome/raw/master/webfonts`,
            destPath,
            fileList: [
                "fa-regular-400.ttf",
                "fa-solid-900.ttf"
            ]
        }),
        downloadFiles({
            srcBaseUrl: `https://github.com/google/material-design-icons/raw/master/font`,
            destPath,
            fileList: [
                "MaterialIcons-Regular.ttf"
            ]
        }),
        downloadFiles({
            srcBaseUrl: `https://github.com/tomochain/tomomaster/raw/master/app/assets/fonts/SFPro`,
            destPath,
            fileList: [
                "sf-pro-text-regular.ttf",
            ]
        }),
        downloadFiles({
            srcBaseUrl: `https://fontsfree.net/wp-content/fonts/basic/sans-serif`,
            destPath,
            fileMap: {
                "FontsFree-Net-SFMono-Regular.ttf": "sf-mono-text-regular.ttf"
            }
        }),
    ]).then(() => undefined);
}

async function download_font_headers() {
    await downloadFiles({
        srcBaseUrl: `https://github.com/juliettef/IconFontCppHeaders/raw/main`,
        destPath: path.join(__dirname, "src/fonts"),
        fileList: [
            "IconsFontAwesome5.h",
            "IconsMaterialDesign.h"
        ]
    });
}

export default {clean, fetch};