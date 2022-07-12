import {path} from "ekx/deps.ts"
import {downloadFiles, getModuleDir, rm} from "ekx/utils/mod.ts"

const __dirname = getModuleDir(import.meta);

async function clean() {
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

export default {clean, fetch};
