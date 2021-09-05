import * as path from "path";
import {execute, executeAsync} from "../utils";
import {resolveFrom} from "../utility/resolveFrom";

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

function getBinaryPath() {
    const ekcPath = path.dirname(resolveFrom(__dirname, "@ekx/ekc/package.json"));
    return path.resolve(ekcPath, "bin", getBinaryOS(), "ekc");
}

export function ekc(...args: string[]): number {
    const bin = getBinaryPath();
    return execute(bin, args);
}

export function ekcAsync(...args: string[]): Promise<number> {
    const bin = getBinaryPath();
    return executeAsync(bin, args);
}