import * as path from "path";
import {downloadFiles, getModuleDir, rm} from "../../modules/utils/mod.js";

const __dirname = getModuleDir(import.meta);

async function clean() {
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

export default {clean, fetch};