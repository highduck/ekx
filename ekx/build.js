const {build} = require("cmake-build");

const buildTypes = ["Release"];
const osTypes = [process.platform];
// const osTypes = ["darwin", "windows", "ios", "android", "web"];

async function run() {
    for (const os of osTypes) {
        for (const buildType of buildTypes) {
            await build({
                os,
                definitions: {
                    EKX_BUILD_TESTS: "ON"
                },
                // enable tests
                test: true
            });
        }
    }
}

run().catch(_ => process.exit(1));