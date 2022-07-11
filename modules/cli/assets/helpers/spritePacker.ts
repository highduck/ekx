import {executeAsync} from "../../utils.ts";
import {getBinaryPath} from "../../utility/bin.ts";

export function spritePackerAsync(configPath: string): Promise<number> {
    const bin = getBinaryPath("@ekx/ekx/tools/sprite-packer", "sprite-packer");
    return executeAsync(bin, [configPath]);
}