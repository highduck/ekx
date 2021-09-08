const {build} = require("cmake-build");
const {spawnSync} = require("child_process");

const buildTypes = ["Release"];
const osTypes = [process.platform];

// const osTypes = ["darwin", "windows", "ios", "android", "web"];

function runTest(dir) {
    console.info("Test", dir);
    const result = spawnSync("ninja", ["test"], {
        stdio: 'inherit',
        cwd: dir,
        env: Object.assign({}, process.env, {CTEST_OUTPUT_ON_FAILURE: "TRUE"})
    });
    if (result.status !== 0) {
        process.exit(result.status);
    }
}

async function run() {
    for (const os of osTypes) {
        for (const buildType of buildTypes) {
            const result = await build({
                os,
                definitions: {
                    EKX_BUILD_TESTS: "ON"
                },
                ninja: true,
                clean: true
            });
            if (os === process.platform || os === "web") {
                runTest(result.buildDir);
            }
        }
    }
}

run().catch(() => process.exit(-1));