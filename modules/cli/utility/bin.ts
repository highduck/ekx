import * as fs from "fs";
import * as path from "path";
import * as os from "os";
import {getModuleDir} from "../../utils/utils.js";

export function getToolsBinPath(bin: string): string {
    if (os.platform() === "win32") {
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
