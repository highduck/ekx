import * as path from "path";
import {resolveFrom} from "./resolveFrom";

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

export function getBinaryPath(pkg: string, bin: string) {
    const rr = resolveFrom(__dirname, pkg + "/package.json");
    if(rr == null) {
        throw new Error("can't find " + pkg + " " + bin);
    }
    const packageDir = path.dirname(rr);
    if (process.platform === "win32") {
        bin += ".exe";
    }
    return path.resolve(packageDir, "bin", getBinaryOS(), bin);
}