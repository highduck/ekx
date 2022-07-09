import {path} from "ekx/deps.ts"
import {downloadFiles, getModuleDir, rm} from "ekx/utils/mod.ts"
import {buildMatrix} from "ekx/cmake/mod.ts"

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
