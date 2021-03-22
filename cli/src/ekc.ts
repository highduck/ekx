import * as path from "path";
import {Project} from "./project";
import {execute, executeAsync} from "./utils";

function getBinaryOS(): string {
    const platform = process.platform;
    switch (platform) {
        case "darwin":
            return "osx";
        case "linux":
            return "linux";
        case "win32":
            return "win32";
    }
    throw new Error(`error: platform ${platform} is not supported`);
}

function getBinaryPath(ctx:Project) {
    return path.join(ctx.path.EKX_ROOT, "ekc/bin", getBinaryOS(), "ekc");
}

export function ekc(ctx: Project, ...args: string[]) {
    const bin = getBinaryPath(ctx);
    execute(bin, args);
}

export function ekcAsync(ctx: Project, ...args: string[]) {
    const bin = getBinaryPath(ctx);
    return executeAsync(bin, args);
}