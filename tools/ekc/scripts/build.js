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
        "-G", "Ninja",
        `-DCMAKE_BUILD_TYPE=${buildType}`,
        ...args,
        "-S", ".",
        "-B", `build/${buildType.toLowerCase()}`,
    ], {
        stdio: 'inherit'
    });
    if(result.status !== 0) {
        process.exit(result.status);
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
        process.exit(result.status);
    }
}

/////// cleaning up
console.info("Remove build folder");

fs.rmSync('build', {recursive: true});

////// permissions
function getExecutablePath() {
    switch(process.platform) {
        case "darwin":
            return "bin/osx/ekc";
        case "win32":
            return "bin/win32/ekc.exe";
        case "linux":
            return "bin/linux/ekc";
    }
    console.error("Platform is not supported: " + process.platform);
    throw "Platform is not supported";
}

const executablePath = getExecutablePath();

console.info("Set executable permissions", executablePath);

fs.chmodSync(executablePath, 0o755);

console.info("Done");
