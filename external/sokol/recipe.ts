import {path, fs} from "../../modules/deps.ts"
import {downloadFiles, getModuleDir, rm} from "../../modules/utils/mod.ts"
import {shdc} from "../../modules/sokol-shdc.ts";

const __dirname = getModuleDir(import.meta);

async function clean() {
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

    // download shdc tool
    await downloadFiles({
        srcBaseUrl: "https://github.com/floooh/sokol-tools-bin/raw/master",
        destPath: __dirname,
        fileList: [
            "bin/linux/sokol-shdc",
            "bin/osx/sokol-shdc",
            "bin/win32/sokol-shdc.exe",
        ]
    });

    Deno.chmodSync(path.join(__dirname, "bin/linux/sokol-shdc"), 0o755);
    Deno.chmodSync(path.join(__dirname, "bin/osx/sokol-shdc"), 0o755);
    Deno.chmodSync(path.join(__dirname, "bin/win32/sokol-shdc.exe"), 0o755);
}

async function test() {
    const testDir = path.join(__dirname, "build");
    try {
        Deno.mkdirSync(testDir, {recursive: true});
    } catch {
        throw new Error("fail to create output directory");
    }

    const r = await shdc({
        input: "test/simple2d.glsl",
        output: path.join(testDir, "simple2d_shader.h"),
        cwd: __dirname
    });

    if (!r) {
        console.warn("sokol-shdc is failed");
    }

    if (!fs.existsSync(path.join(testDir, "simple2d_shader.h"))) {
        throw new Error("shader header not found");
    }

    await rm(testDir);
}

export default {clean, fetch, test};
