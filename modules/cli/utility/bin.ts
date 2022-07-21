import {fs, path} from "../../deps.ts";
import {getModuleDir} from "../../utils/utils.ts";

export function getToolsBinPath(bin: string): string {
    if (Deno.build.os === "windows") {
        bin += ".exe";
    }
    return path.resolve(getModuleDir(import.meta), "../../../tools/bin/" + bin);
}

export function resolveToolsBinPath(bin: string): string {
    const result = getToolsBinPath(bin);
    if (!fs.existsSync(result)) {
        throw new Error("Not found: tools binary @ " + result);
    }
    return result;
}
