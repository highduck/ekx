const fs = require("fs");
const {spawnSync} = require("child_process");

const buildTypes = ["Release"];

for (const buildType of buildTypes) {
    console.info("Generate", buildType);
    spawnSync("cmake", [
        "..",
        "-B", `build/${buildType.toLowerCase()}`,
        "-G", "Ninja",
        `-DCMAKE_BUILD_TYPE=${buildType}`,
        '-DCMAKE_C_COMPILER=clang',
        '-DCMAKE_CXX_COMPILER=clang++'
    ], {
        stdio: 'inherit'
    });
}

for (const buildType of buildTypes) {
    console.info("Build", buildType);
    spawnSync("cmake", [
        "--build", `build/${buildType.toLowerCase()}`,
        "--target", "ekc"
    ], {
        stdio: 'inherit'
    });
}

//fs.rmSync('build', {recursive: true});