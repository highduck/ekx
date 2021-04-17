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

function getBinaryPath(ctx: Project) {
    return path.join(ctx.path.ekc, "bin", getBinaryOS(), "ekc");
}

export function ekc(ctx: Project, ...args: string[]): number {
    const bin = getBinaryPath(ctx);
    return execute(bin, args);
}

export function ekcAsync(ctx: Project, ...args: string[]): Promise<number> {
    const bin = getBinaryPath(ctx);
    return executeAsync(bin, args);
}