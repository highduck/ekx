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
    await downloadFiles({
        srcBaseUrl: "https://github.com/floooh/sokol/raw/master",
        destPath: path.join(__dirname, "include"),
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

async function test() {
    await buildMatrix({
        cmakePath: "test",
        test: true,
        workingDir: __dirname
    });
    await rm(path.join(__dirname, "build"));
}

export default {clean, fetch, test};
