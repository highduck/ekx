import {executeAsync} from "../../utils.js";
import {getOrBuildUtility} from "../../utility/bin.js";

export async function bmfontAsync(configPath: string): Promise<number> {
    const bin = await getOrBuildUtility("bmfont-export");
    return await executeAsync(bin, ["bmfont-export", configPath]);
}