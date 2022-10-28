import {executeAsync} from "../../utils.js";
import {resolveToolsBinPath} from "../../utility/bin.js";

export function spritePackerAsync(configPath: string): Promise<number> {
    const bin = resolveToolsBinPath("sprite-packer");
    return executeAsync(bin, [configPath]);
}