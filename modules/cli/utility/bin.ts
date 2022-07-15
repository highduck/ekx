import {path} from "../../deps.ts";
import {resolveFrom} from "./resolveFrom.ts";
import {getModuleDir} from "../../utils/utils.ts";

function getBinaryOS(): string {
    const platform = Deno.build.os;
    switch (platform) {
        case "darwin":
            return "osx";
        case "linux":
            return "linux";
        case "windows":
            return "win32";
    }
    throw new Error(`error: platform ${platform} is not supported`);
}

export function getBinaryPath(pkg: string, bin: string) {
    if (Deno.build.os === "windows") {
        bin += ".exe";
    }
    const pathToBinary = path.join(pkg, "bin", getBinaryOS(), bin);
    const resolvedBinary = resolveFrom(getModuleDir(import.meta), pathToBinary);
    if (resolvedBinary == null) {
        throw new Error("can't resolve " + pathToBinary);
    }
    return resolvedBinary;
}
