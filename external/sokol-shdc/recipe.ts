import * as path from "https://deno.land/std/path/mod.ts";
import * as fs from "https://deno.land/std/fs/mod.ts";
import {downloadFiles} from "../scripts/download.ts";
import {getModuleDir, rm} from "../scripts/utils.ts";

const __dirname = getModuleDir(import.meta);

async function clean() {
    await rm(path.join(__dirname, "build"));
    await rm(path.join(__dirname, "bin"));
}

async function fetch() {
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
    const testDir = path.join(__dirname, "build/test-shader");
    try {
        Deno.mkdirSync(testDir, {recursive: true});
    } catch {
        throw new Error("fail to create output directory");
    }

    const r = await Deno.run({
        cmd: [
            "node", path.join(__dirname, "sokol-shdc.js"),
            "-i", path.join(__dirname, "test/simple2d.glsl"),
            "-o", path.join(__dirname, "build/test-shader/simple2d_shader.h"),
            "-l", "glsl330:glsl300es:glsl100:hlsl5:metal_ios:metal_sim:metal_macos",
        ],
        stdout: "inherit",
        stderr: "inherit"
    }).status();

    if (!r.success) {
        console.warn("sokol-shdc status code:", r.code);
        //process.exit(1);
    }

    if (!fs.existsSync(path.join(testDir, "simple2d_shader.h"))) {
        throw new Error("shader header not found");
    }

    await rm(testDir);
}

export default {clean, fetch, test};
