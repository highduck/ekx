import {executeAsync} from "../../utils.ts";
import {resolveToolsBinPath} from "../../utility/bin.ts";

export function spritePackerAsync(configPath: string): Promise<number> {
    const bin = resolveToolsBinPath("sprite-packer");
    return executeAsync(bin, [configPath]);
}