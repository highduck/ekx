#!/usr/bin/env -S npx ts-node

import {executeAsync, UtilityConfig} from "../cli/utils.js";
import {logger} from "../cli/logger.js";

UtilityConfig.verbose = true;

// Bump and Build and Upload Game
// ekx android clean --bump patch --deploy internal -v
const release = async (workingDir: string) => {
    logger.info("Build WEB and bump patch version");
    await executeAsync("ekx", ["web", "clean", "--bump", "patch", "--deploy", "internal", "-v"], {workingDir});
    // just build
    // await executeAsync("ekx", ["web", "clean", "-v"], {workingDir});
    // logger.info("Build ANDROID");
    // await executeAsync("ekx", ["android", "clean", "--deploy", "internal", "-v"], {workingDir});
    // logger.info("Build IOS");
    // await executeAsync("ekx", ["ios", "clean", "--deploy", "internal", "-v"], {workingDir});
};

// TODO: add command, use cwd, use game list in package json or arguments

const projects = [
    "../ihtf",
    "../duckstazy",
    "../dice-twice-2048",
    "../colorsense",
    "../ekx-demo",
];

for (const workingDir of projects) {
    await release(workingDir);
}

logger.info("Done!");
