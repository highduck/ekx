const fs = require("fs");
const {build} = require("cmake-build");

const target = "bmfont-export";
function getExecutablePath() {
    switch (process.platform) {
        case "darwin":
            return "bin/osx/" + target;
        case "win32":
            return "bin/win32/" + target + ".exe";
        case "linux":
            return "bin/linux/" + target;
    }
    console.error("Platform is not supported: " + process.platform);
    throw "Platform is not supported";
}

async function run() {
    const buildTypes = ["Release"];
    const osTypes = [process.platform];
    // const osTypes = ["darwin", "windows"];
    for (const os of osTypes) {
        for (const buildType of buildTypes) {
            await build({
                buildType,
                os,
                ninja: true,
                target: target
            });
        }
    }

    ////// permissions
    const executablePath = getExecutablePath();
    console.info("Set executable permissions", executablePath);
    fs.chmodSync(executablePath, 0o755);

    /////// cleaning up
    // console.info("Remove build folder");
    // fs.rmSync('build', {recursive: true});

    console.info("Done");
}

run().catch(_ => process.exit(1));
