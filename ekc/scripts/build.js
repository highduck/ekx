const fs = require("fs");
const {spawnSync} = require("child_process");

const buildTypes = ["Release"];

for (const buildType of buildTypes) {
    console.info("Generate", buildType);
    const args = [];
    if (process.env.USE_CCACHE) {
        args.push("-DCMAKE_C_COMPILER_LAUNCHER=ccache", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache");
    }
    spawnSync("cmake", [
        ".",
        "-B", `build/${buildType.toLowerCase()}`,
        "-G", "Ninja",
        `-DCMAKE_BUILD_TYPE=${buildType}`,
        ...args
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

fs.rmSync('build', {recursive: true});