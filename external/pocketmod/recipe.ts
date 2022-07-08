import * as path from "https://deno.land/std/path/mod.ts";
import {downloadFiles} from "../scripts/download.ts";
import {getModuleDir, rm} from "../scripts/utils.ts";
import {buildMatrix} from "../scripts/cmake.ts";

const __dirname = getModuleDir(import.meta);

async function clean() {
    // TODO:
    // await rm(path.join(__dirname, "build"));
    // await rm(path.join(__dirname, "include"));
}

async function fetch() {
    // TODO:
    // await downloadFiles({
    //     srcBaseUrl: "https://github.com/zeux/pugixml/raw/master",
    //     destPath: path.join(__dirname, "include"),
    //     fileList: [
    //         "src/pugiconfig.hpp",
    //         "src/pugixml.hpp",
    //         "src/pugixml.cpp",
    //     ]
    // });
}

function test() {
    // TODO:
    // return buildMatrix({
    //     cmakePath: "test",
    //     test: true,
    //     workingDir: __dirname
    // });
}

export default {clean, fetch, test};
