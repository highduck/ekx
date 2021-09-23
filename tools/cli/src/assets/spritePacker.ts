import {executeAsync} from "../utils";
import {getBinaryPath} from "../utility/bin";

export function spritePackerAsync(configPath: string): Promise<number> {
    const bin = getBinaryPath("@ekx/sprite-packer", "sprite-packer");
    return executeAsync(bin, [configPath]);
}