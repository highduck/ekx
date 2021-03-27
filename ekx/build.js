const fs = require("fs");
const {spawnSync} = require("child_process");

const buildTypes = ["Release"];

for (const buildType of buildTypes) {
    console.info("Generate", buildType);
    spawnSync("cmake", [
        ".",
        "-B", `build/${buildType.toLowerCase()}`,
        "-G", "Ninja",
        `-DCMAKE_BUILD_TYPE=${buildType}`
    ], {
        stdio: 'inherit'
    });
}

for (const buildType of buildTypes) {
    console.info("Build", buildType);
    spawnSync("cmake", [
        "--build", `build/${buildType.toLowerCase()}`
    ], {
        stdio: 'inherit'
    });
}

fs.rmSync('build', {recursive: true});