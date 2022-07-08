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
    const repoUrl = `https://github.com/mackron/dr_libs/raw/${branch}`;
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: path.join(__dirname, "include/dr"),
        fileList: [
            "dr_flac.h",
            "dr_mp3.h",
            "dr_wav.h"
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
