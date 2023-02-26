import * as path from "path"
import {readTextFileSync, rm, writeTextFileSync} from "../../modules/utils/utils.js"
import {downloadFiles} from "../../modules/utils/download.js"
import {resolveEkxPath} from "../../modules/utils/cacheDir.js";

const destPath = resolveEkxPath("external/imgui");

async function clean() {
    await rm(path.join(destPath, "src"));
    await rm(path.join(destPath, "ttf"));
    await rm(path.join(destPath, "include"));
}

async function fetch() {
    const imgui_branch = "docking";
    await downloadFiles({
        srcBaseUrl: `https://github.com/ocornut/imgui/raw/${imgui_branch}`,
        destPath: path.join(destPath, "src"),
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

` + readTextFileSync(path.join(destPath, "src/imconfig.h"));
    writeTextFileSync(path.join(destPath, "src/imconfig.h"), imgui_config);

    const implot_branch = "master";
    await downloadFiles({
        srcBaseUrl: `https://github.com/epezent/implot/raw/${implot_branch}`,
        destPath: path.join(destPath, "src"),
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
        destPath: path.join(destPath, "src/ImGuizmo"),
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
        srcBaseUrl: `https://github.com/ocornut/imgui_club/raw/main`,
        destPath: path.join(destPath, "src"),
        fileList: [
            "imgui_memory_editor/imgui_memory_editor.h",
        ]
    });

    await download_fonts();
    await download_font_headers();
}

function download_fonts(): Promise<void> {
    const destPath_ = path.join(destPath, "ttf");
    return Promise.all([
        downloadFiles({
            srcBaseUrl: `https://github.com/ocornut/imgui/raw/docking`,
            destPath: destPath_,
            fileMap: {
                "misc/fonts/Cousine-Regular.ttf": "Cousine-Regular.ttf"
            }
        }),
        downloadFiles({
            srcBaseUrl: `https://github.com/FortAwesome/Font-Awesome/raw/master/webfonts`,
            destPath: destPath_,
            fileList: [
                "fa-regular-400.ttf",
                "fa-solid-900.ttf"
            ]
        }),
        downloadFiles({
            srcBaseUrl: `https://github.com/google/material-design-icons/raw/master/font`,
            destPath: destPath_,
            fileList: [
                "MaterialIcons-Regular.ttf"
            ]
        }),
        downloadFiles({
            srcBaseUrl: `https://github.com/tomochain/tomomaster/raw/master/app/assets/fonts/SFPro`,
            destPath: destPath_,
            fileList: [
                "sf-pro-text-regular.ttf",
            ]
        }),
        downloadFiles({
            srcBaseUrl: `https://fontsfree.net/wp-content/fonts/basic/sans-serif`,
            destPath: destPath_,
            fileMap: {
                "FontsFree-Net-SFMono-Regular.ttf": "sf-mono-text-regular.ttf"
            }
        }),
    ]).then(() => undefined);
}

async function download_font_headers() {
    await downloadFiles({
        srcBaseUrl: `https://github.com/juliettef/IconFontCppHeaders/raw/main`,
        destPath: path.join(destPath, "src/fonts"),
        fileList: [
            "IconsFontAwesome5.h",
            "IconsMaterialDesign.h"
        ]
    });
}

export default {clean, fetch};