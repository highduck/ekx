import * as path from "https://deno.land/std/path/mod.ts";
import {downloadFiles} from "../scripts/download.ts";
import {getModuleDir, rm} from "../scripts/utils.ts";
import {buildMatrix} from "../scripts/cmake.ts";

const __dirname = getModuleDir(import.meta);

async function clean() {
    await rm(path.join(__dirname, "build"));
    await rm(path.join(__dirname, "src"));
}

async function fetch() {
    await downloadFiles({
        srcBaseUrl: "https://github.com/zeux/pugixml/raw/master",
        destPath: __dirname,
        fileList: [
            "src/pugiconfig.hpp",
            "src/pugixml.hpp",
            "src/pugixml.cpp",
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
