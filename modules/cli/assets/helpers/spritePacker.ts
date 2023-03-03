import {executeAsync} from "../../utils.js";
import {getOrBuildUtility} from "../../utility/bin.js";

export async function spritePackerAsync(configPath: string): Promise<number> {
    const bin = await getOrBuildUtility("sprite-packer");
    return await executeAsync(bin, ["sprite-packer", configPath]);
}