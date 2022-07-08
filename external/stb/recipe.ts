import * as path from "https://deno.land/std/path/mod.ts";
import {downloadFiles} from "../scripts/download.ts";
import {getModuleDir, rm} from "../scripts/utils.ts";
import {buildMatrix} from "../scripts/cmake.ts";

const __dirname = getModuleDir(import.meta);

async function clean() {
    await rm(path.join(__dirname, "build"));
    await rm(path.join(__dirname, "include"));
}

async function fetch() {
    const branch = "master";
    const repoUrl = `https://github.com/nothings/stb/raw/${branch}`;
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: path.join(__dirname, "include/stb"),
        fileList: [
            "stb_c_lexer.h",
            "stb_connected_components.h",
            "stb_divide.h",
            "stb_ds.h",
            "stb_dxt.h",
            "stb_easy_font.h",
            "stb_herringbone_wang_tile.h",
            "stb_hexwave.h",
            "stb_image.h",
            "stb_image_resize.h",
            "stb_image_write.h",
            "stb_include.h",
            "stb_leakcheck.h",
            "stb_rect_pack.h",
            "stb_sprintf.h",
            "stb_textedit.h",
            "stb_tilemap_editor.h",
            "stb_truetype.h",
            "stb_vorbis.c",
            "stb_voxel_render.h"
        ]
    });

    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: __dirname,
        fileList: ["LICENSE"]
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
