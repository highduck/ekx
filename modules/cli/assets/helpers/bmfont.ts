import {executeAsync} from "../../utils.ts";
import {resolveToolsBinPath} from "../../utility/bin.ts";

export function bmfontAsync(configPath: string): Promise<number> {
    const bin = resolveToolsBinPath("bmfont-export");
    return executeAsync(bin, [configPath]);
}