import {executeAsync} from "../../utils.js";
import {resolveToolsBinPath} from "../../utility/bin.js";

export function bmfontAsync(configPath: string): Promise<number> {
    const bin = resolveToolsBinPath("bmfont-export");
    return executeAsync(bin, [configPath]);
}