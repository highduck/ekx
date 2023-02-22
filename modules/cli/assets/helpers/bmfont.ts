import {executeAsync} from "../../utils.js";
import {getOrBuildToolBinary} from "../../utility/bin.js";

export async function bmfontAsync(configPath: string): Promise<number> {
    const bin = await getOrBuildToolBinary("bmfont-export");
    return await executeAsync(bin, [configPath]);
}