import {fs, path} from "../../modules/deps.ts"
import {download, downloadFiles, getModuleDir, rm} from "../../modules/utils/mod.ts"
import {shdc} from "../../modules/sokol-shdc.ts";
import {getToolsBinPath} from "../../modules/cli/utility/bin.ts";

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
    const exeURL = {
        linux: "bin/linux/sokol-shdc",
        darwin: "bin/osx/sokol-shdc",
        windows: "bin/win32/sokol-shdc.exe",
    }[Deno.build.os];
    const exePath = getToolsBinPath("sokol-shdc");
    fs.ensureDirSync(path.dirname(exePath));
    await download("https://github.com/floooh/sokol-tools-bin/raw/master/" + exeURL, exePath);
    Deno.chmodSync(exePath, 0o755);
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
