const {downloadFiles} = require('@ekx/cli-utils');

async function run() {

    console.info("dr-libs");

    const branch = "master";
    const repoUrl = `https://github.com/mackron/dr_libs/raw/${branch}`;
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: "include/dr",
        fileList: [
            "dr_flac.h",
            "dr_mp3.h",
            "dr_wav.h"
        ]
    });

    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: ".",
        fileList: ["LICENSE"]
    });
}

run().catch(() => process.exit(1));