import {executeAsync} from "../../utils";
import {getBinaryPath} from "../../utility/bin";

export function bmfontAsync(configPath: string): Promise<number> {
    const bin = getBinaryPath("@ekx/bmfont-export", "bmfont-export");
    return executeAsync(bin, [configPath]);
}