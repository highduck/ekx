import {executeAsync} from "../../utils.ts";
import {getBinaryPath} from "../../utility/bin.ts";

export function bmfontAsync(configPath: string): Promise<number> {
    const bin = getBinaryPath("@ekx/ekx/tools/bmfont-export", "bmfont-export");
    return executeAsync(bin, [configPath]);
}