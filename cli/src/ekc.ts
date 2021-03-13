import * as path from "path";
import {Project} from "./project";
import {execute, executeAsync, isDir, rimrafAsync} from "./utils";
import * as fs from "fs";
import rimraf = require("rimraf");

export function ekc(ctx: Project, ...args: string[]) {
    const bin = path.join(ctx.path.EKX_ROOT, "dev-tools/bin/ekc");
    execute(bin, args);
}

export function ekcAsync(ctx: Project, ...args: string[]) {
    const bin = path.join(ctx.path.EKX_ROOT, "dev-tools/bin/ekc");
    return executeAsync(bin, args);
}

async function build_temp_cmake(buildType: string) {
    let tempDir = `../_temp_build/${buildType}`;
    if (!isDir(tempDir)) {
        await fs.promises.mkdir(tempDir, {recursive: true});
    }
    console.info("Generate:", buildType);
    await executeAsync("cmake", ["../..", "-GNinja", `-DCMAKE_BUILD_TYPE=${buildType}`], {workingDir: tempDir});
    console.info("Build:", "ekc", buildType);
    await executeAsync("ninja", ["ekc"], {workingDir: tempDir});
    await rimrafAsync(tempDir);
    console.info("Clean:", tempDir);
}

export function rebuild_ekc() {
    return Promise.all([
        build_temp_cmake("Debug"),
        build_temp_cmake("Release")
    ]);
}