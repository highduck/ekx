const fs = require("fs");
const {spawnSync} = require("child_process");

const buildTypes = ["Release"];

for (const buildType of buildTypes) {
    console.info("Generate", buildType);
    const args = [];
    if (process.env.USE_CCACHE) {
        args.push("-DCMAKE_C_COMPILER_LAUNCHER=ccache", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache");
    }
    const result = spawnSync("cmake", [
        ".",
        "-B", `build/${buildType.toLowerCase()}`,
        "-G", "Ninja",
        `-DCMAKE_BUILD_TYPE=${buildType}`,
        ...args
    ], {
        stdio: 'inherit'
    }).status;
    if(result.status !== 0) {
        process.exit(1);
    }
}

for (const buildType of buildTypes) {
    console.info("Build", buildType);
    const result = spawnSync("cmake", [
        "--build", `build/${buildType.toLowerCase()}`,
        "--target", "ekc"
    ], {
        stdio: 'inherit'
    });
    if(result.status !== 0) {
        process.exit(1);
    }
}

fs.rmSync('build', {recursive: true});