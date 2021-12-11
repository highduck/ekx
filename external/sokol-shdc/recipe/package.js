const {chmodSync} = require("fs");
const {downloadFiles} = require('@ekx/cli-utils');

async function run() {
    console.info("sokol-shdc binaries");
    await downloadFiles({
        srcBaseUrl: "https://github.com/floooh/sokol-tools-bin/raw/master",
        fileList: [
            "bin/linux/sokol-shdc",
            "bin/osx/sokol-shdc",
            "bin/win32/sokol-shdc.exe",
        ]
    });

    chmodSync("bin/linux/sokol-shdc", 0o755);
    chmodSync("bin/osx/sokol-shdc", 0o755);
    chmodSync("bin/win32/sokol-shdc.exe", 0o755);
}

run().catch(() => process.exit(1));