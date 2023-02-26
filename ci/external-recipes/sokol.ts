import * as path from "path";
import {rm} from "../../modules/utils/utils.js";
import {downloadFiles} from "../../modules/utils/download.js";
import {resolveEkxPath} from "../../modules/utils/cacheDir.js";

const destPath = resolveEkxPath("external/sokol");

async function clean() {
    await rm(path.join(destPath, "include"));
}

async function fetch() {
    await downloadFiles({
        srcBaseUrl: "https://github.com/floooh/sokol/raw/master",
        destPath: path.join(destPath, "include"),
        fileList: [
            "sokol_app.h",
            "sokol_args.h",
            "sokol_audio.h",
            "sokol_fetch.h",
            "sokol_gfx.h",
            "sokol_glue.h",
            "sokol_time.h",
            "util/sokol_debugtext.h",
            "util/sokol_fontstash.h",
            "util/sokol_gfx_imgui.h",
            "util/sokol_gl.h",
            "util/sokol_imgui.h",
            "util/sokol_memtrack.h",
            "util/sokol_shape.h",
        ]
    });
}

export default {clean, fetch};
