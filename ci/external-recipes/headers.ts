import * as path from "path";
import {rm} from "../../modules/utils/utils.js";
import {downloadFiles} from "../../modules/utils/download.js";
import {resolveEkxPath} from "../../modules/utils/cacheDir.js";

const destPath = resolveEkxPath("external/headers");

async function clean() {
    await rm(path.join(destPath, "include"));
}

function fetch() {
    return Promise.all([
        download_stb(),
        download_dr(),
        download_pocketmod()
    ]).then(_ => undefined);
}

async function download_stb() {
    const branch = "master";
    const repoUrl = `https://github.com/nothings/stb/raw/${branch}`;
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: path.join(destPath, "include/stb"),
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
}

async function download_dr() {
    const branch = "master";
    const repoUrl = `https://github.com/mackron/dr_libs/raw/${branch}`;
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: path.join(destPath, "include/dr"),
        fileList: [
            "dr_flac.h",
            "dr_mp3.h",
            "dr_wav.h"
        ]
    });
}

async function download_pocketmod() {
    const branch = "master";
    const repoUrl = `https://github.com/rombankzero/pocketmod/raw/${branch}`;
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: path.join(destPath, "include/pocketmod"),
        fileList: [
            "pocketmod.h"
        ]
    });
}

export default {clean, fetch};
