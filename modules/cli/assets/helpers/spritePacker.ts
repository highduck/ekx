import {executeAsync} from "../../utils.js";
import {getOrBuildToolBinary} from "../../utility/bin.js";

export async function spritePackerAsync(configPath: string): Promise<number> {
    const bin = await getOrBuildToolBinary("sprite-packer");
    return await executeAsync(bin, [configPath]);
}